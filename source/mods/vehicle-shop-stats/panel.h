/* Reuse the garage's own sockets and TMP styles; never write its description. */
#define VS_METRICS 7
#define VS_OWNED 9
#define VS_RECTS 20
typedef struct {DRoot rect;float position[2],size[2];} VSRect;
static struct {
 PTR transform,go_transform,parent,object,clone,child_count,child,active,is_active,destroy,position,size,set_position,set_size,font,minfont,maxfont,autosize,wrapping,overflow,raycast;
 DRoot menu,objects[VS_OWNED],values[VS_METRICS];VSRect saved[VS_RECTS];
 int owned,saved_count,shown,model,failed;
} vp;
static PTR vs_reference(PTR method,PTR self){int ok=1;if(!alive(self))return 0;PTR value=call(method,self,0,&ok);return ok&&alive(value)?value:0;}
static PTR vs_obj(PTR component){return vs_reference(vp.object,component);}
static PTR vs_transform(PTR component){return vs_reference(vp.transform,component);}
static PTR vs_go_transform(PTR object){return vs_reference(vp.go_transform,object);}
static int vs_active(PTR object,int enabled){int ok=1;unsigned char on=(unsigned char)enabled;void *a[]={&on};if(!alive(object))return 0;call(vp.active,object,a,&ok);return ok;}
static int vs_place(PTR rect,float x,float y,float width,float height){
 int ok=1;float p[]={x,-y},s[]={width,height};void *a[]={p},*b[]={s};if(!alive(rect))return 0;call(vp.set_position,rect,a,&ok);call(vp.set_size,rect,b,&ok);return ok;
}
static int vs_save(PTR rect){
 if(!alive(rect)||vp.saved_count>=VS_RECTS)return 0;VSRect *s=&vp.saved[vp.saved_count];int ok=1;
 if(!stats_vector(vp.position,rect,s->position,2,&ok)||!stats_vector(vp.size,rect,s->size,2,&ok))return 0;
 s->rect=dhold(rect);if(!s->rect)return 0;vp.saved_count++;return 1;
}
static void vs_restore(void){
 for(int i=0;i<vp.saved_count;i++){
  VSRect *s=&vp.saved[i];PTR rect=dget(s->rect);int ok=1;void *a[]={s->position},*b[]={s->size};
  if(alive(rect)){call(vp.set_position,rect,a,&ok);call(vp.set_size,rect,b,&ok);}ddrop(&s->rect);
 }vp.saved_count=0;
}
static void vs_hide(void){if(!vp.shown&&!vp.saved_count)return;vs_restore();for(int i=0;i<vp.owned;i++)vs_active(dget(vp.objects[i]),0);vp.shown=0;vp.model=-1;event("stats-panel-hidden");}
static void vs_drop(void){
 vs_hide();int ok=1;for(int i=0;i<vp.owned;i++){PTR obj=dget(vp.objects[i]);void *a[]={obj};if(alive(obj))call(vp.destroy,0,a,&ok);ddrop(&vp.objects[i]);}
 for(int i=0;i<VS_METRICS;i++)ddrop(&vp.values[i]);ddrop(&vp.menu);vp.owned=vp.failed=0;vp.model=-1;
}
static PTR vs_copy(PTR original,PTR parent){
 if(!alive(original)||!alive(parent)||vp.owned>=VS_OWNED)return 0;int ok=1;unsigned char world=0;void *a[]={original,parent,&world};PTR object=call(vp.clone,0,a,&ok);
 if(!ok||!alive(object))return 0;DRoot root=dhold(object);
 if(!root){vs_active(object,0);void *destroy[]={object};call(vp.destroy,0,destroy,&ok);return 0;}
 vp.objects[vp.owned++]=root;if(!vs_active(object,0))return 0;return object;
}
static int vs_style(PTR value,float font){
 int ok=1;unsigned char off=0;int wrap=0,overflow=0;void *a[]={&font},*b[]={&off},*c[]={&wrap},*o[]={&overflow};
 call(vp.autosize,value,b,&ok);call(vp.font,value,a,&ok);call(vp.wrapping,value,c,&ok);call(vp.overflow,value,o,&ok);call(vp.raycast,value,b,&ok);return ok;
}
static PTR vs_text_component(PTR transform){int ok=1;void *a[]={dtype("TMPro","TMP_Text")};if(!alive(transform)||!a[0])return 0;PTR value=call(component_m,transform,a,&ok);return ok&&alive(value)?value:0;}
static PTR vs_text_copy(PTR original,PTR parent,const char *value,float x,float y,float width,float height,float font){
 PTR go=vs_copy(vs_obj(original),parent);if(!go)return 0;PTR component=vs_text_component(vs_go_transform(go));
 if(!component||!vs_place(vs_transform(component),x,y,width,height)||!vs_style(component,font)||!text(component,value))return 0;return component;
}
static int vs_create(PTR menu){
 PTR rows=member(menu,"statRows"),captions=member(menu,"statCaptions"),values=member(menu,"statValues");
 if(!rows||!captions||!values||da.array_length(rows)!=4||da.array_length(captions)!=4||da.array_length(values)!=4)return 0;
 int ok=1;PTR row=stats_item(rows,0);
 /* Use the native Details transform, which already follows canvas scaling. */
 PTR root_tr=vs_reference(vp.parent,vs_go_transform(row));
 if(!root_tr||!ok)return 0;vp.menu=dhold(menu);if(!vp.menu)return 0;
 if(!vs_text_copy(member(menu,"category"),root_tr,"VEHICLE SPECIFICATIONS",32,232,520,20,14))return 0;
 static const char *labels[]={"BASE SPEED","STORAGE","CARGO SLOTS","TURN","BODY WIDTH","BODY LENGTH","HEFT"};
 for(int i=0;i<VS_METRICS;i++){
  PTR copy=vs_copy(row,root_tr);if(!copy)return 0;
  PTR row_transform=vs_go_transform(copy),texts[2]={0};int found=0,count=stats_integer(vp.child_count,row_transform,&ok);
  if(!ok||count<2||count>16)return 0;
  for(int child=0;child<count;child++){void *a[]={&child};PTR transform=call(vp.child,row_transform,a,&ok),component=vs_text_component(transform);if(component){if(found==2)return 0;texts[found++]=component;}}
  if(!ok||found!=2)return 0;PTR caption=texts[0],value=texts[1];
  /* Child traversal order is checked by its native caption/value position. */
  float first[2],second[2];
  if(!stats_vector(vp.position,vs_transform(caption),first,2,&ok)||!stats_vector(vp.position,vs_transform(value),second,2,&ok))return 0;
  float first_x=first[0],second_x=second[0];
  if(!ok||first_x==second_x)return 0;if(first_x>second_x){PTR swap=caption;caption=value;value=swap;}
  if(!vs_place(vs_go_transform(copy),32+(i%2)*268.f,258+(i/2)*44.f,252,40)||
   !vs_place(vs_transform(caption),12,9,106,26)||!vs_style(caption,12)||!text(caption,labels[i]))return 0;
  /* The native font needs a full row: short boxes ellipsize vertically
   * even when preferredWidth is far below the available width. */
  if(!vs_place(vs_transform(value),115,0,125,40)||!vs_style(value,18))return 0;
  /* Keep long values and units inside their cell without moving the grid. */
  float minimum=12,maximum=18;unsigned char fit=1;void *lo[]={&minimum},*hi[]={&maximum},*on[]={&fit};
  call(vp.minfont,value,lo,&ok);call(vp.maxfont,value,hi,&ok);call(vp.autosize,value,on,&ok);
  if(!ok)return 0;vp.values[i]=dhold(value);if(!vp.values[i])return 0;
 }
 if(!vs_text_copy(member(menu,"summary"),root_tr,"Catalog values; upgrades and boost excluded",32,438,520,26,13))return 0;
 return 1;
}
/* SetButtons recalculates these during native Update. Reconcile after Update,
 * before canvas rendering, and avoid dirtying unchanged RectTransforms. Native
 * buttons use centered pivots; preserve two-button widths when both are shown. */
static int vs_fit(PTR rect,float x,float y,float w,float h){
 if(!alive(rect))return 0;int ok=1;float v[2];if(!stats_vector(vp.position,rect,v,2,&ok))return 0;
 int changed=v[0]!=x||v[1]!=-y;
 if(!stats_vector(vp.size,rect,v,2,&ok))return 0;
 return changed||v[0]!=w||v[1]!=h?vs_place(rect,x,y,w,h):1;
}
static int vs_controls(void){
 PTR menu=dget(vp.menu);if(!vp.shown||vp.saved_count!=15||!alive(menu)||int_field(menu,"page")!=vehicles_page)return 0;
 int ok=1;PTR second=member(menu,"second");if(!alive(second))return 0;PTR object=vs_obj(second);if(!alive(object))return 0;int both=stats_boolean(vp.is_active,object,&ok);
 if(!ok)return 0;float w=both?252:520;
 return vs_fit(dget(vp.saved[12].rect),32+w*.5f,632,w,56)&&
  vs_fit(dget(vp.saved[13].rect),426,632,252,56)&&
  vs_fit(dget(vp.saved[14].rect),32,674,520,80);
}
static int vs_show(PTR menu){
 PTR rows=member(menu,"statRows"),captions=member(menu,"statCaptions"),values=member(menu,"statValues");
 if(!rows||!captions||!values||da.array_length(rows)!=4||da.array_length(captions)!=4||da.array_length(values)!=4||vp.saved_count)return 0;
 /* Capture the whole native layout before changing any rect. */
 for(int i=0;i<4;i++){
  PTR row=stats_item(rows,(U64)i),caption=stats_item(captions,(U64)i),value=stats_item(values,(U64)i);
  if(!alive(row)||!alive(caption)||!alive(value)||!vs_save(vs_go_transform(row))||!vs_save(vs_transform(caption))||!vs_save(vs_transform(value)))return 0;
 }
 const char *fields[]={"action","second","status"};for(int i=0;i<3;i++)if(!vs_save(vs_transform(member(menu,fields[i]))))return 0;
 for(int i=0;i<4;i++)if(!vs_place(dget(vp.saved[i*3].rect),32,470+i*30.f,520,26)||
  !vs_place(dget(vp.saved[i*3+1].rect),18,4,208,22)||!vs_place(dget(vp.saved[i*3+2].rect),156,0,348,26))return 0;
 for(int i=0;i<vp.owned;i++)if(!vs_active(dget(vp.objects[i]),1))return 0;
 vp.shown=1;return vs_controls();
}
static int vs_valid(void){
 if(vp.owned!=VS_OWNED||!alive(dget(vp.menu)))return 0;
 for(int i=0;i<vp.owned;i++)if(!alive(dget(vp.objects[i])))return 0;
 for(int i=0;i<VS_METRICS;i++)if(!alive(dget(vp.values[i])))return 0;
 return 1;
}
static int vs_bind(void){
#define VS(F,NS,T,N,S) vp.F=dmethod(NS,T,N,S);if(!vp.F){event("missing-" T "-" N);return 0;}
 VS(transform,"UnityEngine","Component","get_transform","");VS(go_transform,"UnityEngine","GameObject","get_transform","");VS(parent,"UnityEngine","Transform","get_parent","");VS(object,"UnityEngine","Component","get_gameObject","");
 VS(clone,"UnityEngine","Object","Instantiate","UnityEngine.Object;UnityEngine.Transform;System.Boolean");VS(child_count,"UnityEngine","Transform","get_childCount","");VS(child,"UnityEngine","Transform","GetChild","System.Int32");
 VS(is_active,"UnityEngine","GameObject","get_activeSelf","");VS(active,"UnityEngine","GameObject","SetActive","System.Boolean");VS(destroy,"UnityEngine","Object","Destroy","UnityEngine.Object");
 VS(position,"UnityEngine","RectTransform","get_anchoredPosition","");VS(size,"UnityEngine","RectTransform","get_sizeDelta","");
 VS(set_position,"UnityEngine","RectTransform","set_anchoredPosition","UnityEngine.Vector2");VS(set_size,"UnityEngine","RectTransform","set_sizeDelta","UnityEngine.Vector2");
 VS(font,"TMPro","TMP_Text","set_fontSize","System.Single");VS(autosize,"TMPro","TMP_Text","set_enableAutoSizing","System.Boolean");VS(minfont,"TMPro","TMP_Text","set_fontSizeMin","System.Single");VS(maxfont,"TMPro","TMP_Text","set_fontSizeMax","System.Single");VS(wrapping,"TMPro","TMP_Text","set_textWrappingMode","TMPro.TextWrappingModes");VS(overflow,"TMPro","TMP_Text","set_overflowMode","TMPro.TextOverflowModes");
 VS(raycast,"UnityEngine.UI","Graphic","set_raycastTarget","System.Boolean");
#undef VS
 return 1;
}
