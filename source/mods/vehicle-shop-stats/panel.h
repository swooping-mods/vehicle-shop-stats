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
static PTR vs_obj(PTR component){int ok=1;return call(vp.object,component,0,&ok);}
static PTR vs_transform(PTR component){int ok=1;return call(vp.transform,component,0,&ok);}
static PTR vs_go_transform(PTR object){int ok=1;return call(vp.go_transform,object,0,&ok);}
static void vs_active(PTR object,int enabled){int ok=1;unsigned char on=(unsigned char)enabled;void *a[]={&on};if(object)call(vp.active,object,a,&ok);}
static void vs_place(PTR rect,float x,float y,float width,float height){
 int ok=1;float p[]={x,-y},s[]={width,height};void *a[]={p},*b[]={s};call(vp.set_position,rect,a,&ok);call(vp.set_size,rect,b,&ok);
}
static int vs_save(PTR rect){
 if(!rect||vp.saved_count>=VS_RECTS)return 0;VSRect *s=&vp.saved[vp.saved_count];int ok=1;
 PTR p=call(vp.position,rect,0,&ok);if(!ok||!p)return 0;dcopy(s->position,unbox(p),8);
 PTR size=call(vp.size,rect,0,&ok);if(!ok||!size)return 0;dcopy(s->size,unbox(size),8);
 s->rect=dhold(rect);if(!s->rect)return 0;vp.saved_count++;return 1;
}
static void vs_restore(void){
 for(int i=0;i<vp.saved_count;i++){
  VSRect *s=&vp.saved[i];PTR rect=dget(s->rect);int ok=1;void *a[]={s->position},*b[]={s->size};
  call(vp.set_position,rect,a,&ok);call(vp.set_size,rect,b,&ok);ddrop(&s->rect);
 }vp.saved_count=0;
}
static void vs_hide(void){if(!vp.shown&&!vp.saved_count)return;vs_restore();for(int i=0;i<vp.owned;i++)vs_active(dget(vp.objects[i]),0);vp.shown=0;vp.model=-1;event("stats-panel-hidden");}
static void vs_drop(void){
 vs_hide();int ok=1;for(int i=0;i<vp.owned;i++){PTR obj=dget(vp.objects[i]);void *a[]={obj};if(obj)call(vp.destroy,0,a,&ok);ddrop(&vp.objects[i]);}
 for(int i=0;i<VS_METRICS;i++)ddrop(&vp.values[i]);ddrop(&vp.menu);vp.owned=vp.failed=0;vp.model=-1;
}
static PTR vs_copy(PTR original,PTR parent){
 if(!original||!parent||vp.owned>=VS_OWNED)return 0;int ok=1;unsigned char world=0;void *a[]={original,parent,&world};PTR object=call(vp.clone,0,a,&ok);
 if(!ok||!object)return 0;DRoot root=dhold(object);if(!root)return 0;vp.objects[vp.owned++]=root;vs_active(object,0);return object;
}
static void vs_style(PTR value,float font){
 int ok=1;unsigned char off=0;int wrap=0,overflow=0;void *a[]={&font},*b[]={&off},*c[]={&wrap},*o[]={&overflow};
 call(vp.autosize,value,b,&ok);call(vp.font,value,a,&ok);call(vp.wrapping,value,c,&ok);call(vp.overflow,value,o,&ok);call(vp.raycast,value,b,&ok);
}
static PTR vs_text_component(PTR transform){int ok=1;void *a[]={dtype("TMPro","TMP_Text")};return call(component_m,transform,a,&ok);}
static PTR vs_text_copy(PTR original,PTR parent,const char *value,float x,float y,float width,float height,float font){
 PTR go=vs_copy(vs_obj(original),parent);if(!go)return 0;PTR component=vs_text_component(vs_go_transform(go));
 if(!component)return 0;vs_place(vs_transform(component),x,y,width,height);vs_style(component,font);text(component,value);return component;
}
static int vs_create(PTR menu){
 PTR rows=member(menu,"statRows"),captions=member(menu,"statCaptions"),values=member(menu,"statValues");
 if(!rows||!captions||!values||da.array_length(rows)!=4||da.array_length(captions)!=4||da.array_length(values)!=4)return 0;
 int ok=1;PTR row=*(PTR*)darray(rows,0);
 /* Use the native Details transform, which already follows canvas scaling. */
 PTR root_tr=call(vp.parent,vs_go_transform(*(PTR*)darray(rows,0)),0,&ok);
 if(!root_tr||!ok)return 0;vp.menu=dhold(menu);
 if(!vs_text_copy(member(menu,"category"),root_tr,"VEHICLE SPECIFICATIONS",32,232,520,20,14))return 0;
 static const char *labels[]={"BASE SPEED","STORAGE","CARGO SLOTS","TURN","BODY WIDTH","BODY LENGTH","HEFT"};
 for(int i=0;i<VS_METRICS;i++){
  PTR copy=vs_copy(row,root_tr);if(!copy)return 0;
  PTR row_transform=vs_go_transform(copy),texts[2]={0};int found=0,count=integer(vp.child_count,row_transform,&ok);
  if(!ok||count<2||count>16)return 0;
  for(int child=0;child<count;child++){void *a[]={&child};PTR transform=call(vp.child,row_transform,a,&ok),component=vs_text_component(transform);if(component){if(found==2)return 0;texts[found++]=component;}}
  if(!ok||found!=2)return 0;PTR caption=texts[0],value=texts[1];
  /* Child traversal order is checked by its native caption/value position. */
  PTR boxed=call(vp.position,vs_transform(caption),0,&ok);float first_x=boxed?*(float*)unbox(boxed):0;
  boxed=call(vp.position,vs_transform(value),0,&ok);float second_x=boxed?*(float*)unbox(boxed):0;
  if(!ok||first_x==second_x)return 0;if(first_x>second_x){PTR swap=caption;caption=value;value=swap;}
  vs_place(vs_go_transform(copy),32+(i%2)*268.f,258+(i/2)*44.f,252,40);
  vs_place(vs_transform(caption),12,9,106,26);vs_style(caption,12);text(caption,labels[i]);
  /* The native font needs a full row: short boxes ellipsize vertically
   * even when preferredWidth is far below the available width. */
  vs_place(vs_transform(value),115,0,125,40);vs_style(value,18);
  /* Keep long values and units inside their cell without moving the grid. */
  float minimum=12,maximum=18;unsigned char fit=1;void *lo[]={&minimum},*hi[]={&maximum},*on[]={&fit};
  call(vp.minfont,value,lo,&ok);call(vp.maxfont,value,hi,&ok);call(vp.autosize,value,on,&ok);
  vp.values[i]=dhold(value);if(!vp.values[i])return 0;
 }
 if(!vs_text_copy(member(menu,"summary"),root_tr,"Catalog values; upgrades and boost excluded",32,438,520,26,13))return 0;
 return 1;
}
/* SetButtons recalculates these during native Update. Reconcile after Update,
 * before canvas rendering, and avoid dirtying unchanged RectTransforms. Native
 * buttons use centered pivots; preserve two-button widths when both are shown. */
static void vs_fit(PTR rect,float x,float y,float w,float h){
 int ok=1;PTR p=call(vp.position,rect,0,&ok);if(!ok||!p)return;
 float *v=(float*)unbox(p);int changed=v[0]!=x||v[1]!=-y;
 PTR size=call(vp.size,rect,0,&ok);if(!ok||!size)return;v=(float*)unbox(size);
 if(changed||v[0]!=w||v[1]!=h)vs_place(rect,x,y,w,h);
}
static void vs_controls(void){
 PTR menu=dget(vp.menu);if(!menu||int_field(menu,"page")!=vehicles_page)return;
 int ok=1;PTR second=member(menu,"second");int both=boolean(vp.is_active,vs_obj(second),&ok);
 if(!ok)return;float w=both?252:520;
 vs_fit(dget(vp.saved[12].rect),32+w*.5f,632,w,56);
 vs_fit(dget(vp.saved[13].rect),426,632,252,56);
 vs_fit(dget(vp.saved[14].rect),32,674,520,80);
}
static int vs_show(PTR menu){
 PTR rows=member(menu,"statRows"),captions=member(menu,"statCaptions"),values=member(menu,"statValues");if(!rows||!captions||!values)return 0;
 for(int i=0;i<4;i++){
  PTR row=vs_go_transform(*(PTR*)darray(rows,i)),caption=vs_transform(*(PTR*)darray(captions,i)),value=vs_transform(*(PTR*)darray(values,i));
  if(!vs_save(row)||!vs_save(caption)||!vs_save(value))return 0;
  vs_place(row,32,470+i*30.f,520,26);vs_place(caption,18,4,208,22);vs_place(value,156,0,348,26);
 }
 const char *fields[]={"action","second","status"};for(int i=0;i<3;i++){
  PTR rect=vs_transform(member(menu,fields[i]));if(!vs_save(rect))return 0;

 }
 for(int i=0;i<vp.owned;i++)vs_active(dget(vp.objects[i]),1);vp.shown=1;vs_controls();return 1;
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
