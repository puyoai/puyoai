#ifndef HAMAJI_MT19937INT_H_
#define HAMAJI_MT19937INT_H_

#define MT19937_N 624

class MT {

 private:
  unsigned long mt[MT19937_N]; /* the array for the state vector  */
  int mti; /* mti==MT19937_N+1 means mt[MT19937_N] is not initialized */
  
 public:
  explicit MT(unsigned long s);
  
  /* generates a random number on [0,0xffffffff]-interval */
  unsigned long genrand_int32(void);

};

#endif  // HAMAJI_MT19937INT_H_
