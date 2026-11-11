/* I15: stable specifications in their own native garage sockets. */
#define MOONLAB_LATE_UPDATE_ROLE MOON_ROLE_OBSERVER
#include "base.h"
#include "panel.h"
static PTR vehicles_f,storage_m,heft_m;static int test_mode,stats_fixture_id;static unsigned ticks;
static void stats_value(int row,float value,const char *suffix){char b[100];char *p=decimal_text(b,value);cat(p,suffix);text(dget(vp.values[row]),b);}
static void stats_fill(PTR spec,int model){
 int ok=1;void *args[]={&model};double kg=0;float heft=0;PTR boxed=call(storage_m,0,args,&ok);if(ok&&boxed)dcopy(&kg,unbox(boxed),8);
 boxed=call(heft_m,0,args,&ok);if(ok&&boxed)dcopy(&heft,unbox(boxed),4);
 stats_value(0,float_field(spec,"speed")*3.6f," km/h");stats_value(1,(float)kg," kg");
 char b[80];int capacity=int_field(spec,"capacity");num(b,(U64)(capacity>0?capacity:0));text(dget(vp.values[2]),b);
 stats_value(3,float_field(spec,"turn"),"");stats_value(4,float_field(spec,"width")," m");stats_value(5,float_field(spec,"depth")," m");stats_value(6,heft,"");
}
static void stats_geometry(void){
 if(!test_mode||!log_file)return;
 for(int row=0;row<2;row++){
  PTR value=dget(vp.values[row]);int ok=1;char b[3000],*p=cat(b,"{\"event\":\"text-geometry\",\"row\":");p=num(p,(U64)row);
  const char *names[]={"get_fontSize","get_preferredWidth","get_fontSizeMin","get_fontSizeMax"};
  for(int i=0;i<4;i++){float f=dnumber(dmethod("TMPro","TMP_Text",names[i],""),value,&ok);p=cat(p,",\"");p=cat(p,names[i]);p=cat(p,"\":");p=decimal_text(p,f);}
  const char *ints[]={"get_overflowMode","get_alignment","get_firstVisibleCharacter","get_maxVisibleCharacters","get_maxVisibleLines"};
  for(int i=0;i<5;i++){int n=integer(dmethod("TMPro","TMP_Text",ints[i],""),value,&ok);p=cat(p,",\"");p=cat(p,ints[i]);p=cat(p,"\":");p=num(p,(U64)n);}
  const char *vectors[]={"get_margin","get_rect","get_pivot","get_anchorMin","get_anchorMax"};
  for(int i=0;i<5;i++){PTR boxed=call(dmethod(i?"UnityEngine":"TMPro",i?"RectTransform":"TMP_Text",vectors[i],""),i?vs_transform(value):value,0,&ok);p=cat(p,",\"");p=cat(p,vectors[i]);p=cat(p,"\":[");float *f=boxed?(float*)unbox(boxed):0;int count=i<2?4:2;for(int j=0;j<count;j++){if(j)p=cat(p,",");p=decimal_text(p,f?f[j]:0);}p=cat(p,"]");}
  p=cat(p,"}\n");DWORD n;WriteFile(log_file,b,(DWORD)(p-b),&n,0);
 }
}
static void stats_sample(PTR menu,int model){
 if(!test_mode||!log_file)return;int ok=1;PTR get=dmethod("TMPro","TMP_Text","get_text","");PTR s=call(get,member(menu,"summary"),0,&ok);unsigned hash=2166136261u;
 if(s){const unsigned short *c=string_chars(s);int length=string_length(s);for(int i=0;i<length;i++){hash^=c[i];hash*=16777619u;}}
 char b[320],*p=cat(b,"{\"event\":\"stats-sample\",\"model\":");p=num(p,(U64)model);p=cat(p,",\"description_hash\":");p=num(p,hash);p=cat(p,",\"metrics\":7,\"shown\":");p=num(p,(U64)vp.shown);p=cat(p,"}\n");DWORD written;WriteFile(log_file,b,(DWORD)(p-b),&written,0);
}
/* Diagnostic-only native garage record in the task-owned, restored world.
 * A fresh save has no built garage; the real menu still requires IsGarage(). */
static PTR stats_test_menu(PTR walker){
 int ok=1;void *a[]={dtype("MoonCleaners","MoonGarageMenu")};PTR menu=call(component_m,walker,a,&ok);
 if(!menu){PTR go=vs_obj(walker);menu=call(dmethod("UnityEngine","GameObject","AddComponent","System.Type"),go,a,&ok);event("test-menu-component-added");}
 return menu;
}
static void stats_test_open(PTR walker,PTR menu,int id){
 int ok=1;unsigned char off=0;void *release[]={&off};
 /* The new-world cutscene can retain these flags after its visible UI closes. */
 call(dmethod("MoonCleaners","MoonWalker","set_Held","System.Boolean"),walker,release,&ok);
 call(dmethod("MoonCleaners","MoonWalker","set_Travelling","System.Boolean"),walker,release,&ok);
 call(dmethod("MoonCleaners","MoonWalker","set_UIBlocked","System.Boolean"),walker,release,&ok);
 void *a[]={&id};PTR opened=call(dmethod("MoonCleaners","MoonGarageMenu","Open","System.Int32"),menu,a,&ok);
 PTR world=call(dmethod("MoonCleaners","MoonGarageMenu","get_World",""),menu,0,&ok);
 int blocked=boolean(dmethod("MoonCleaners","MoonWalker","get_UIBlocked",""),walker,&ok),building=boolean(dmethod("MoonCleaners","MoonWalker","get_Building",""),walker,&ok),mounted=boolean(dmethod("MoonCleaners","MoonWalker","get_Mounted",""),walker,&ok),restoring=world?boolean(dmethod("MoonCleaners","MoonWorld","get_IsRestoringRendering",""),world,&ok):-1;
 char b[400],*p=cat(b,"{\"event\":\"test-open-result\",\"ok\":");p=num(p,(U64)ok);p=cat(p,",\"opened\":");p=num(p,opened&&*(unsigned char*)unbox(opened)?1:0);p=cat(p,",\"blocked\":");p=num(p,(U64)blocked);p=cat(p,",\"building\":");p=num(p,(U64)building);p=cat(p,",\"mounted\":");p=num(p,(U64)mounted);p=cat(p,",\"restoring\":");p=num(p,(U64)(restoring<0?9:restoring));p=cat(p,"}\n");DWORD n;WriteFile(log_file,b,(DWORD)(p-b),&n,0);
}
static void stats_test_fixture(PTR walker){
 PTR world=member(walker,"world"),depots=member(world,"depots");if(!world||!depots)return;int ok=1,kind=0,id=3000000;
 PTR field=get_field(find_class("MoonCleaners","MoonWorld"),"GarageKind");if(!field)return;static_field(field,&kind);
 PTR module=GetModuleHandleA("GameAssembly.dll");void (*set)(PTR,PTR,void*)=(void(*)(PTR,PTR,void*))GetProcAddress(module,"il2cpp_field_set_value");if(!set)return;
 for(int tries=0;tries<16;tries++,id++){void *a[]={&id};PTR used=call(get_method(object_class(depots),"ContainsKey",1),depots,a,&ok);if(!ok||!used)return;if(!*(unsigned char*)unbox(used))break;}
 PTR snapshot=da.new_object(find_class("MoonCleaners","MoonDepotSnapshot"));DRoot hold=dhold(snapshot);if(!hold)return;call(get_method(object_class(snapshot),".ctor",0),snapshot,0,&ok);
 set(snapshot,get_field(object_class(snapshot),"id"),&id);set(snapshot,get_field(object_class(snapshot),"kind"),&kind);
 /* The native save writer requires an owner string even for this UI record. */
 int (*offset)(PTR)=(int(*)(PTR))GetProcAddress(module,"il2cpp_field_get_offset");
 void (*barrier)(PTR,PTR*,PTR)=(void(*)(PTR,PTR*,PTR))GetProcAddress(module,"il2cpp_gc_wbarrier_set_field");
 PTR owner=call(dmethod("MoonCleaners","MoonCrew","get_LocalPlayerId",""),0,0,&ok);
 PTR owner_field=get_field(object_class(snapshot),"owner");int owner_offset=offset&&owner_field?offset(owner_field):0;
 if(!ok||!owner||!barrier||owner_offset<16||owner_offset>96){ddrop(&hold);event("test-fixture-owner-failed");return;}
 barrier(snapshot,(PTR*)((char*)snapshot+owner_offset),owner);
 void *add[]={&id,snapshot};call(get_method(object_class(depots),"Add",2),depots,add,&ok);ddrop(&hold);
 void *a[]={&id};PTR valid=call(dmethod("MoonCleaners","MoonWorld","IsGarage","System.Int32"),world,a,&ok);
 if(ok&&valid&&*(unsigned char*)unbox(valid)){
  PTR menu=stats_test_menu(walker);stats_test_open(walker,menu,id);event("test-native-garage-fixture");
 }else event("test-garage-fixture-failed");
 stats_fixture_id=id;
}
static void stats_test_cleanup(PTR walker){
 if(!stats_fixture_id||garage_menu(walker))return;
 PTR depots=member(member(walker,"world"),"depots");if(!depots)return;
 int ok=1;void *a[]={&stats_fixture_id};PTR removed=call(get_method(object_class(depots),"Remove",1),depots,a,&ok);
 event(ok&&removed&&*(unsigned char*)unbox(removed)?"test-garage-fixture-removed":"test-garage-fixture-remove-failed");stats_fixture_id=0;
}

static void stats_test(PTR walker){
 stats_test_cleanup(walker);static int was_open,was_next,was_page,pending;int open=down(0x75),next=down(0x76),page=down(0x77),ok=1;
 if(open&&!was_open)pending=1;
 int ready=1;PTR w=member(walker,"world");if(w)ready=!boolean(dmethod("MoonCleaners","MoonWorld","get_IsRestoringRendering",""),w,&ok);else ready=0;
 if(pending&&ready&&ok){pending=0;
  event("test-open-request");int inactive=1,sort=0;void *a[]={dtype("MoonCleaners","MoonGarage"),&inactive,&sort};PTR list=call(dmethod("UnityEngine","Object","FindObjectsByType","System.Type;UnityEngine.FindObjectsInactive;UnityEngine.FindObjectsSortMode"),0,a,&ok);
  if(list&&ok&&da.array_length(list)>0){PTR garage=*(PTR*)darray(list,0);int id=integer(method("MoonCleaners","MoonGarage","get_Id",0),garage,&ok);void *show[]={&id};PTR menu=0;void *component[]={dtype("MoonCleaners","MoonGarageMenu")};menu=call(component_m,walker,component,&ok);call(dmethod("MoonCleaners","MoonGarageMenu","Open","System.Int32"),menu,show,&ok);event("test-garage-opened");}
  if(!garage_menu(walker)){
   PTR world=member(walker,"world"),is_garage=dmethod("MoonCleaners","MoonWorld","IsGarage","System.Int32"),menu=0;void *component[]={dtype("MoonCleaners","MoonGarageMenu")};menu=call(component_m,walker,component,&ok);
   for(int id=0;world&&menu&&id<4096;id++){void *a[]={&id};PTR found=call(is_garage,world,a,&ok);if(ok&&found&&*(unsigned char*)unbox(found)){call(dmethod("MoonCleaners","MoonGarageMenu","Open","System.Int32"),menu,a,&ok);event("test-garage-opened-from-world");break;}}
  }
  if(!garage_menu(walker))stats_test_fixture(walker);
 }
 PTR menu=garage_menu(walker);if(menu&&next&&!was_next){PTR cards=member(menu,"cards"),items=member(cards,"_items");int count=int_field(cards,"_size"),index=-1;PTR selected=member(menu,"selected");if(items&&count>0&&(U64)count<=da.array_length(items)){for(int i=0;i<count;i++)if(*(PTR*)darray(items,(U64)i)==selected)index=i;PTR card=*(PTR*)darray(items,(U64)((index+1)%count));int model=int_field(card,"vehicle");void *a[]={&model};call(dmethod("MoonCleaners","MoonGarageMenu","PickModel","System.Int32"),menu,a,&ok);event("test-next-vehicle");}}
 if(menu&&page&&!was_page){int current=int_field(menu,"page"),target=current==vehicles_page?fleet_page:current==fleet_page?upgrades_page:vehicles_page;unsigned char sound=0;void *a[]={&target,&sound};call(dmethod("MoonCleaners","MoonGarageMenu","ShowPage","MoonCleaners.MoonGarageMenu.Page;System.Boolean"),menu,a,&ok);event("test-page-changed");}
 was_open=open;was_next=next;was_page=page;
}
static int before(int id,Context *c){
 if(!on_thread())return 0;if(id==1){if(vp.shown)vs_controls();return 0;}static int seen;if(test_mode&&!seen){event("test-callback-active");seen=1;}PTR walker=(PTR)c->rcx;if(test_mode)stats_test(walker);PTR menu=garage_menu(walker);
 if(!menu){vs_hide();return 0;}
 if(menu!=dget(vp.menu)){vs_drop();if(!vs_create(menu)){vs_drop();vp.menu=dhold(menu);vp.failed=1;event("stats-panel-create-failed");}}
 if(vp.failed)return 0;if(int_field(menu,"page")!=vehicles_page){vs_hide();return 0;}
 PTR selected=member(menu,"selected");if(!selected){vs_hide();return 0;}int model=int_field(selected,"vehicle");PTR array=0;static_field(vehicles_f,&array);
 if(!array||model<0||(U64)model>=da.array_length(array)){vs_hide();return 0;}PTR spec=*(PTR*)darray(array,(U64)model);if(!spec){vs_hide();return 0;}
 if(!vp.shown&&!vs_show(menu)){vs_drop();vp.menu=dhold(menu);vp.failed=1;event("stats-panel-layout-failed");return 0;}
 if(model!=vp.model){stats_fill(spec,model);vp.model=model;ticks=0;event("stats-panel-selected");}
 if(++ticks%12==0)stats_sample(menu,model);if(ticks==12)stats_geometry();return 0;
}
__declspec(dllexport) int MoonLab_ModStart(PTR domain,PTR game){
 if(!bind_base(domain,game,"vehicle-shop-stats.jsonl")||!vs_bind())return 0;
 char test[8];test_mode=GetEnvironmentVariableA("MOONLAB_STATS_TEST",test,sizeof(test))==1&&test[0]=='1';vp.model=-1;if(test_mode)event("test-enabled");
 vehicles_f=get_field(find_class("MoonCleaners","MoonPrototypeCatalog"),"Vehicles");storage_m=dmethod("MoonCleaners","MoonPrototypeCatalog","StorageKg","System.Int32");heft_m=dmethod("MoonCleaners","MoonPrototypeCatalog","Heft","System.Int32");
 if(!vehicles_f||!storage_m||!heft_m||!prepare_hook(method("MoonCleaners","MoonWalker","Update",0),100671733,0,game)||!prepare_hook(method("MoonCleaners","MoonWalker","LateUpdate",0),100671741,1,game))return 0;before_hook=before;event("ready-separate-statistics");return install_hooks();
}
