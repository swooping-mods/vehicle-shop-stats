static PTR vehicles_f,storage_m,heft_m;
static int stats_value(int row,float value,const char *suffix){char b[100];char *p=decimal_text(b,value);cat(p,suffix);return text(dget(vp.values[row]),b);}
static int stats_fill(PTR spec,int model){
 int ok=1,capacity=-1;void *args[]={&model};double kg=0;float heft=0,speed=0,turn=0,width=0,depth=0;
 stats_payload(call(storage_m,0,args,&ok),&kg,sizeof(kg),&ok);
 stats_payload(call(heft_m,0,args,&ok),&heft,sizeof(heft),&ok);
 ok=stats_field(spec,"speed",&speed)&&stats_field(spec,"capacity",&capacity)&&
  stats_field(spec,"turn",&turn)&&stats_field(spec,"width",&width)&&stats_field(spec,"depth",&depth)&&ok;
 /* Validate doubles before narrowing; validate converted speed before a float
  * multiplication could overflow. Failure never becomes a catalog zero. */
 double kmh=(double)speed*3.6;
 if(!ok||capacity<0||!stats_metric(kg)||!stats_metric(kmh)||!stats_metric(heft)||
  !stats_metric(turn)||!stats_metric(width)||!stats_metric(depth)){
  for(int i=0;i<VS_METRICS;i++)text(dget(vp.values[i]),"?");return 0;
 }
 int written=stats_value(0,(float)kmh," km/h");written=stats_value(1,(float)kg," kg")&&written;
 char b[80];num(b,(U64)capacity);written=text(dget(vp.values[2]),b)&&written;
 written=stats_value(3,turn,"")&&written;written=stats_value(4,width," m")&&written;
 written=stats_value(5,depth," m")&&written;return stats_value(6,heft,"")&&written;
}
/* Only a complete successful write can suppress a later retry. */
static int stats_select(PTR spec,int model){
 if(!stats_fill(spec,model)){vp.model=-1;return 0;}vp.model=model;return 1;
}
