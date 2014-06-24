#ifndef PERIA_CONNCTOR_H_
#define PERIA_CONNCTOR_H_

namespace peria {

struct Game;

class Connector {
 public:
  Connector();
  ~Connector();

  bool Receive(Game* game);
  void Send();
};

}  // namespace peria

#endif  // PERIA_CONNCTOR_H_
