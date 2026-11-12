/* I15: stable specifications in their own native garage sockets. */
#define MOONLAB_LATE_UPDATE_ROLE MOON_ROLE_OBSERVER
#include "base.h"
#include "panel.h"
#include "values.h"
static int test_mode,stats_fixture_id;static unsigned ticks;static DRoot stats_fixture_depots;
static void stats_geometry(void){
 if(!test_mode||!log_file)return;
 for(int row=0;row<2;row++){
  PTR value=dget(vp.values[row]);int ok=1;char b[3000],*p=cat(b,"{\"event\":\"text-geometry\",\"row\":");p=num(p,(U64)row);
  const char *names[]={"get_fontSize","get_preferredWidth","get_fontSizeMin","get_fontSizeMax"};
  for(int i=0;i<4;i++){float f=stats_number(dmethod("TMPro","TMP_Text",names[i],""),value,&ok);p=cat(p,",\"");p=cat(p,names[i]);p=cat(p,"\":");p=ok&&f>=-10000000.f&&f<=10000000.f?decimal_text(p,f):cat(p,"null");}
  const char *ints[]={"get_overflowMode","get_alignment","get_firstVisibleCharacter","get_maxVisibleCharacters","get_maxVisibleLines"};
  for(int i=0;i<5;i++){int n=stats_integer(dmethod("TMPro","TMP_Text",ints[i],""),value,&ok);p=cat(p,",\"");p=cat(p,ints[i]);p=cat(p,"\":");p=ok&&n>=0?num(p,(U64)n):cat(p,"null");}
  const char *vectors[]={"get_margin","get_rect","get_pivot","get_anchorMin","get_anchorMax"};
  for(int i=0;i<5;i++){PTR boxed=call(dmethod(i?"UnityEngine":"TMPro",i?"RectTransform":"TMP_Text",vectors[i],""),i?vs_transform(value):value,0,&ok);p=cat(p,",\"");p=cat(p,vectors[i]);p=cat(p,"\":[");float f[4]={0};int count=i<2?4:2;int valid=stats_payload(boxed,f,(U64)count*sizeof(float),&ok);for(int j=0;j<count;j++){if(j)p=cat(p,",");p=valid&&f[j]>=-10000000.f&&f[j]<=10000000.f?decimal_text(p,f[j]):cat(p,"null");}p=cat(p,"]");}
  p=cat(p,"}\n");DWORD n;WriteFile(log_file,b,(DWORD)(p-b),&n,0);
 }
}
static void stats_sample(PTR menu,int model){
 if(!test_mode||!log_file)return;int ok=1;PTR get=dmethod("TMPro","TMP_Text","get_text","");PTR s=call(get,member(menu,"summary"),0,&ok);unsigned hash=2166136261u;
 if(!ok||!s)return;{const unsigned short *c=string_chars(s);int length=string_length(s);if(!c||length<0)return;for(int i=0;i<length;i++){hash^=c[i];hash*=16777619u;}}
 char b[320],*p=cat(b,"{\"event\":\"stats-sample\",\"model\":");p=num(p,(U64)model);p=cat(p,",\"description_hash\":");p=num(p,hash);p=cat(p,",\"metrics\":7,\"shown\":");p=num(p,(U64)vp.shown);p=cat(p,"}\n");DWORD written;WriteFile(log_file,b,(DWORD)(p-b),&written,0);
}
/* Diagnostic-only native garage record in the task-owned, restored world.
 * A fresh save has no built garage; the real menu still requires IsGarage(). */
static PTR stats_test_menu(PTR walker){
 int ok=1;void *a[]={dtype("MoonCleaners","MoonGarageMenu")};PTR menu=call(component_m,walker,a,&ok);
 if(!ok)return 0;if(!menu){PTR go=vs_obj(walker);if(!go)return 0;menu=call(dmethod("UnityEngine","GameObject","AddComponent","System.Type"),go,a,&ok);event("test-menu-component-added");}
 return ok&&alive(menu)?menu:0;
}
static void stats_test_open(PTR walker,PTR menu,int id){
 int ok=1;if(!alive(walker)||!alive(menu))return;unsigned char off=0;void *release[]={&off};
 /* The new-world cutscene can retain these flags after its visible UI closes. */
 call(dmethod("MoonCleaners","MoonWalker","set_Held","System.Boolean"),walker,release,&ok);if(!ok)return;
 call(dmethod("MoonCleaners","MoonWalker","set_Travelling","System.Boolean"),walker,release,&ok);if(!ok)return;
 call(dmethod("MoonCleaners","MoonWalker","set_UIBlocked","System.Boolean"),walker,release,&ok);if(!ok)return;
 void *a[]={&id};PTR opened=call(dmethod("MoonCleaners","MoonGarageMenu","Open","System.Int32"),menu,a,&ok);
 int opened_state=stats_bool(opened,&ok);
 PTR world=call(dmethod("MoonCleaners","MoonGarageMenu","get_World",""),menu,0,&ok);
 int blocked=stats_boolean(dmethod("MoonCleaners","MoonWalker","get_UIBlocked",""),walker,&ok),building=stats_boolean(dmethod("MoonCleaners","MoonWalker","get_Building",""),walker,&ok),mounted=stats_boolean(dmethod("MoonCleaners","MoonWalker","get_Mounted",""),walker,&ok),restoring=world?stats_boolean(dmethod("MoonCleaners","MoonWorld","get_IsRestoringRendering",""),world,&ok):-1;
 char b[400],*p=cat(b,"{\"event\":\"test-open-result\",\"ok\":");p=num(p,(U64)ok);p=cat(p,",\"opened\":");p=num(p,opened_state);p=cat(p,",\"blocked\":");p=num(p,(U64)blocked);p=cat(p,",\"building\":");p=num(p,(U64)building);p=cat(p,",\"mounted\":");p=num(p,(U64)mounted);p=cat(p,",\"restoring\":");p=num(p,(U64)(restoring<0?9:restoring));p=cat(p,"}\n");DWORD n;WriteFile(log_file,b,(DWORD)(p-b),&n,0);
}
static void stats_test_fixture(PTR walker){
 if(stats_fixture_id)return;PTR world=member(walker,"world"),depots=member(world,"depots");if(!world||!depots)return;int ok=1,kind=0,id=3000000;
 PTR field=get_field(find_class("MoonCleaners","MoonWorld"),"GarageKind");if(!field)return;static_field(field,&kind);
 PTR module=GetModuleHandleA("GameAssembly.dll");void (*set)(PTR,PTR,void*)=(void(*)(PTR,PTR,void*))GetProcAddress(module,"il2cpp_field_set_value");if(!set)return;
 for(int tries=0;tries<16;tries++,id++){void *a[]={&id};PTR used=call(get_method(object_class(depots),"ContainsKey",1),depots,a,&ok);int occupied=stats_bool(used,&ok);if(!ok)return;if(!occupied)break;if(tries==15)return;}
 PTR snapshot_type=find_class("MoonCleaners","MoonDepotSnapshot");if(!snapshot_type)return;PTR snapshot=da.new_object(snapshot_type);DRoot hold=dhold(snapshot);if(!hold)return;call(get_method(object_class(snapshot),".ctor",0),snapshot,0,&ok);
 PTR id_field=get_field(object_class(snapshot),"id"),kind_field=get_field(object_class(snapshot),"kind");
 if(!ok||!id_field||!kind_field){ddrop(&hold);return;}set(snapshot,id_field,&id);set(snapshot,kind_field,&kind);
 /* The native save writer requires an owner string even for this UI record. */
 int (*offset)(PTR)=(int(*)(PTR))GetProcAddress(module,"il2cpp_field_get_offset");
 void (*barrier)(PTR,PTR*,PTR)=(void(*)(PTR,PTR*,PTR))GetProcAddress(module,"il2cpp_gc_wbarrier_set_field");
 PTR owner=call(dmethod("MoonCleaners","MoonCrew","get_LocalPlayerId",""),0,0,&ok);
 PTR owner_field=get_field(object_class(snapshot),"owner");int owner_offset=offset&&owner_field?offset(owner_field):0;
 if(!ok||!owner||!barrier||owner_offset<16||owner_offset>96){ddrop(&hold);event("test-fixture-owner-failed");return;}
 barrier(snapshot,(PTR*)((char*)snapshot+owner_offset),owner);
 stats_fixture_depots=dhold(depots);if(!stats_fixture_depots){ddrop(&hold);return;}
 stats_fixture_id=id;void *add[]={&id,snapshot};call(get_method(object_class(depots),"Add",2),depots,add,&ok);ddrop(&hold);
 if(!ok){event("test-fixture-add-unknown");return;}
 void *a[]={&id};PTR valid=call(dmethod("MoonCleaners","MoonWorld","IsGarage","System.Int32"),world,a,&ok);
 if(stats_bool(valid,&ok)&&ok){
  PTR menu=stats_test_menu(walker);stats_test_open(walker,menu,id);event("test-native-garage-fixture");
 }else event("test-garage-fixture-failed");
 stats_fixture_id=id;
}
static void stats_test_cleanup(PTR walker){
 if(!stats_fixture_id||garage_menu(walker))return;
 /* An unresolved result belongs to its original dictionary, even after a
  * world replacement. Never remove the same numeric ID from another world. */
 PTR depots=dget(stats_fixture_depots);if(!depots)return;
 int ok=1;void *a[]={&stats_fixture_id};PTR removed=call(get_method(object_class(depots),"Remove",1),depots,a,&ok);
 int did_remove=stats_bool(removed,&ok);event(ok&&did_remove?"test-garage-fixture-removed":"test-garage-fixture-remove-failed");if(ok){stats_fixture_id=0;ddrop(&stats_fixture_depots);}
}

static void stats_test(PTR walker){
 stats_test_cleanup(walker);static int was_open,was_next,was_page,pending;int open=down(0x75),next=down(0x76),page=down(0x77),ok=1;
 if(open&&!was_open)pending=1;
 int ready=1;PTR w=member(walker,"world");if(w)ready=!stats_boolean(dmethod("MoonCleaners","MoonWorld","get_IsRestoringRendering",""),w,&ok);else ready=0;
 if(pending&&ready&&ok){pending=0;
  event("test-open-request");int inactive=1,sort=0;void *a[]={dtype("MoonCleaners","MoonGarage"),&inactive,&sort};PTR list=call(dmethod("UnityEngine","Object","FindObjectsByType","System.Type;UnityEngine.FindObjectsInactive;UnityEngine.FindObjectsSortMode"),0,a,&ok);
  if(list&&ok&&da.array_length(list)>0){PTR garage=stats_item(list,0);if(!alive(garage))return;int id=stats_integer(method("MoonCleaners","MoonGarage","get_Id",0),garage,&ok);if(!ok||id<0)return;void *show[]={&id};PTR menu=0;void *component[]={dtype("MoonCleaners","MoonGarageMenu")};menu=call(component_m,walker,component,&ok);call(dmethod("MoonCleaners","MoonGarageMenu","Open","System.Int32"),menu,show,&ok);event("test-garage-opened");}
  if(!garage_menu(walker)){
   PTR world=member(walker,"world"),is_garage=dmethod("MoonCleaners","MoonWorld","IsGarage","System.Int32"),menu=0;void *component[]={dtype("MoonCleaners","MoonGarageMenu")};menu=call(component_m,walker,component,&ok);
   for(int id=0;world&&menu&&id<4096;id++){void *a[]={&id};PTR found=call(is_garage,world,a,&ok);int exists=stats_bool(found,&ok);if(!ok)break;if(exists){call(dmethod("MoonCleaners","MoonGarageMenu","Open","System.Int32"),menu,a,&ok);event("test-garage-opened-from-world");break;}}
  }
  if(!garage_menu(walker))stats_test_fixture(walker);
 }
 PTR menu=garage_menu(walker);if(menu&&next&&!was_next){PTR cards=member(menu,"cards"),items=member(cards,"_items");int count=int_field(cards,"_size"),index=-1;PTR selected=member(menu,"selected");if(items&&count>0&&(U64)count<=da.array_length(items)){for(int i=0;i<count;i++)if(stats_item(items,(U64)i)==selected)index=i;PTR card=stats_item(items,(U64)((index+1)%count));int model=int_field(card,"vehicle");void *a[]={&model};call(dmethod("MoonCleaners","MoonGarageMenu","PickModel","System.Int32"),menu,a,&ok);event("test-next-vehicle");}}
 if(menu&&page&&!was_page){int current=int_field(menu,"page"),target=current==vehicles_page?fleet_page:current==fleet_page?upgrades_page:vehicles_page;unsigned char sound=0;void *a[]={&target,&sound};call(dmethod("MoonCleaners","MoonGarageMenu","ShowPage","MoonCleaners.MoonGarageMenu.Page;System.Boolean"),menu,a,&ok);event("test-page-changed");}
 was_open=open;was_next=next;was_page=page;
}
#include "lifecycle.h"
static int before(int id,Context *c){
 if(!on_thread())return 0;PTR walker=(PTR)c->rcx;if(!stats_admit(walker))return 0;
 if(id==1){stats_late(walker);return 0;}
 static int seen;if(test_mode&&!seen){event("test-callback-active");seen=1;}
 if(test_mode)stats_test(walker);PTR menu=garage_menu(walker);
 if(!menu){if(vp.failed||!alive(dget(vp.menu)))vs_drop();else vs_hide();return 0;}
 if(menu!=dget(vp.menu)||(!vp.failed&&!vs_valid())){vs_drop();if(!vs_create(menu)){vs_drop();vp.menu=dhold(menu);vp.failed=1;event("stats-panel-create-failed");}}
 if(vp.failed)return 0;if(int_field(menu,"page")!=vehicles_page){vs_hide();return 0;}
 PTR selected=member(menu,"selected");if(!selected){vs_hide();return 0;}int model=int_field(selected,"vehicle");PTR array=0;static_field(vehicles_f,&array);
 if(!array||model<0||(U64)model>=da.array_length(array)){vs_hide();return 0;}PTR spec=stats_item(array,(U64)model);if(!spec){vs_hide();return 0;}
 if(!vp.shown&&!vs_show(menu)){vs_drop();vp.menu=dhold(menu);vp.failed=1;event("stats-panel-layout-failed");return 0;}
 if(model!=vp.model){if(!stats_select(spec,model))return 0;ticks=0;event("stats-panel-selected");}
 if(++ticks%12==0)stats_sample(menu,model);if(ticks==12)stats_geometry();return 0;
}
__declspec(dllexport) int MoonLab_ModStart(PTR domain,PTR game){
 if(!bind_base(domain,game,"vehicle-shop-stats.jsonl")||!vs_bind())return 0;
 char test[8];test_mode=GetEnvironmentVariableA("MOONLAB_STATS_TEST",test,sizeof(test))==1&&test[0]=='1';vp.model=-1;if(test_mode)event("test-enabled");
 vehicles_f=get_field(find_class("MoonCleaners","MoonPrototypeCatalog"),"Vehicles");storage_m=dmethod("MoonCleaners","MoonPrototypeCatalog","StorageKg","System.Int32");heft_m=dmethod("MoonCleaners","MoonPrototypeCatalog","Heft","System.Int32");
 if(!vehicles_f||!storage_m||!heft_m||!prepare_hook(method("MoonCleaners","MoonWalker","Update",0),100671733,0,game)||!prepare_hook(method("MoonCleaners","MoonWalker","LateUpdate",0),100671741,1,game))return 0;before_hook=before;event("ready-separate-statistics");return install_hooks();
}
