#ifndef HIT_H
#define HIT_H

class Hit{

 public:

  int adc_charge;
  int pmt_id;
  float time_corse;
  float time_fine;

  static int Size(){ return sizeof(adc_charge)+sizeof(pmt_id)+sizeof(time_corse)+sizeof(time_fine);}

};


#endif
