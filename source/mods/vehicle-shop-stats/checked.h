/* Feature-local checks until the shared adapter revision is qualified. Known
 * pinned value layouts only; a successful call can still have no payload. */
static inline int stats_payload(PTR boxed,void *out,U64 size,int *ok){
 PTR data=*ok&&boxed?unbox(boxed):0;if(!data){*ok=0;return 0;}dcopy(out,data,size);return 1;
}
static inline int stats_bool(PTR boxed,int *ok){
 unsigned char value=0;if(!stats_payload(boxed,&value,1,ok)||value>1){*ok=0;return 0;}return value;
}
static inline int stats_boolean(PTR method,PTR self,int *ok){return stats_bool(call(method,self,0,ok),ok);}
static inline int stats_integer(PTR method,PTR self,int *ok){int value=0;stats_payload(call(method,self,0,ok),&value,sizeof(value),ok);return value;}
static inline float stats_number(PTR method,PTR self,int *ok){float value=0;stats_payload(call(method,self,0,ok),&value,sizeof(value),ok);return value;}
static inline int stats_alive(PTR method,PTR object){int ok=1;void *args[]={object};PTR boxed=object?call(method,0,args,&ok):0;return stats_bool(boxed,&ok)&&ok;}
static inline int stats_field(PTR object,const char *name,void *out){
 PTR type=object?object_class(object):0,field=type?get_field(type,name):0;
 if(!field)return 0;field_value(object,field,out);return 1;
}
static inline PTR stats_item(PTR array,U64 index){
 PTR slot=array?darray(array,index):0,value=0;if(slot)dcopy(&value,slot,sizeof(value));return value;
}
/* Ordered bounds reject NaN and infinities without a CRT dependency. The text
 * formatter's existing upper bound also keeps its integer conversion safe. */
static inline int stats_metric(double value){return value>=0&&value<=10000000.0;}
static inline int stats_vector(PTR method,PTR self,float *out,int count,int *ok){
 float value[4]={0};if(count<1||count>4){*ok=0;return 0;}if(!stats_payload(call(method,self,0,ok),value,(U64)count*sizeof(float),ok))return 0;
 for(int i=0;i<count;i++)if(!(value[i]>=-10000000.f&&value[i]<=10000000.f)){*ok=0;return 0;}
 dcopy(out,value,(U64)count*sizeof(float));return 1;
}
