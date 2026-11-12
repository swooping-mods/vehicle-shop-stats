static DRoot stats_actor,stats_world;
static void stats_reset(void){vs_drop();ddrop(&stats_actor);ddrop(&stats_world);}
static int stats_admit(PTR walker){
 PTR local=0;if(!moon_local_actor_read(&local)||!local){stats_reset();return 0;}if(walker!=local)return 0;
 PTR world=member(walker,"world");int ok=1;
 if(!world||stats_boolean(restoring_m,world,&ok)||!ok){stats_reset();return 0;}
 if(walker!=dget(stats_actor)||world!=dget(stats_world)){
  stats_reset();stats_actor=dhold(walker);stats_world=dhold(world);
  if(!stats_actor||!stats_world){stats_reset();return 0;}
 }return 1;
}
static void stats_late(PTR walker){
 PTR menu=garage_menu(walker);
 if(!menu||menu!=dget(vp.menu)||int_field(menu,"page")!=vehicles_page){vs_hide();return;}
 if(vp.shown&&(!vs_valid()||!vs_controls()))vs_hide();
}
