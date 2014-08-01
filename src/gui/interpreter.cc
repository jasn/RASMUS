#include <interpreter.hh>
#include <memory>
#include <iostream>

#include "frontend/callback.hh"
#include "frontend/interpreter.hh"

namespace rf = rasmus::frontend;

class GuiCallBack : public rasmus::frontend::Callback {
public:


  virtual void report(rf::MsgType type, 
		      std::shared_ptr<rf::Code> code,
		      std::string message,
		      rf::Token mainToken,
		      std::vector<rf::CharRange> ranges) override {
    std::cout << message << std::endl;
  }


  virtual void report(rf::MsgType type, std::string message) override {
    std::cout << message << std::endl;
  }

  virtual void print(Type type, std::string repr) override {
    std::cout << repr << std::endl;
  }

  virtual void saveRelation(rm_object * o, const char * name) override {}
  virtual rm_object * loadRelation(const char * name) override { return 0; }
  virtual bool hasRelation(const char * name) override { return true; }


};

class InterpreterPrivate {

public:
  std::shared_ptr<rasmus::frontend::Interperter> interpreter;
  std::shared_ptr<rasmus::frontend::Callback> callback;

  InterpreterPrivate() {
    int options = 0; // dump no info on terminal.
    callback = std::make_shared<GuiCallBack>();
    interpreter=rasmus::frontend::makeInterperter(callback);
    interpreter->setup(options, std::string("Interpreter"));    
  }
};

Interpreter::Interpreter(QObject *parent) : QObject(parent) {
  d_ptr = new InterpreterPrivate();
}

Interpreter::~Interpreter() {
  delete d_ptr;
}

void Interpreter::run(QString line) {
  d_ptr->interpreter->runLine(line.toUtf8().data());
  
  if (d_ptr->interpreter->complete()) {
    emit complete();
  } else {
    emit incomplete();
  }

}

