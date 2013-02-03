#ifndef __OJAMA_CONTROLLER_H__
#define __OJAMA_CONTROLLER_H__

#include <vector>

class OjamaController {
 public:
  OjamaController();
  void SetOpponent(OjamaController* opponent);
  int GetFixedOjama() const;
  int GetPendingOjama() const;
  int GetYokokuOjama() const { return GetFixedOjama() + GetPendingOjama(); }
  void Send(int num);
  void Fix();
  std::vector<int> Drop();

  static OjamaController DUMMY;

 private:
  OjamaController* opponent_;
  int fixed_;
  int pending_;
};

#endif  // __OJAMA_CONTROLLER_H__
