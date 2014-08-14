#include <interpreter.hh>
#include <memory>
#include <iostream>
#include <algorithm>

#include "frontend/callback.hh"
#include "frontend/interpreter.hh"

namespace rf = rasmus::frontend;

struct escaped {
public:
  const char * str;
  explicit escaped(const char * str): str(str) {}
  explicit escaped(const std::string & str): str(str.c_str()) {}
  
  friend std::ostream & operator<<(std::ostream & o, const escaped & e) {
    const char * c=e.str;
    while (true) {
      switch (*c) {
      case '<': o << "&lt;"; break;
      case '>': o << "&ge;"; break;
      case ' ': o << "&nbsp;"; break;
      case '"': o << "&qout;"; break;
      case '\'': o << "&apos;"; break;
      case '&': o << "&amp;" ; break;
      case '\n': o << "<br>" ; break;
      case '\0': return o;
      default: o << *c; break;
      }
      ++c;
    }
    return o;
  }  
};


class GuiCallBack : public rasmus::frontend::Callback {
public:
  GuiCallBack(Interpreter * interperter): interperter(interperter) {}

  void environmentChanged(const char * name) override {
    interperter->environmentChanged(name);
  }

  virtual void report(rf::MsgType type, 
		      std::shared_ptr<rf::Code> code,
		      std::string message,
		      rf::Token mainToken,
		      std::vector<rf::CharRange> ranges) override {
    	int lo = std::numeric_limits<int>::max();
	int hi = std::numeric_limits<int>::min();
	if (mainToken) {
		lo = std::min<int>(lo, mainToken.start);
		hi = std::max<int>(hi, mainToken.length + mainToken.start);
	}
	for (auto r: ranges) {
		lo = std::min<int>(lo, r.lo);
		hi = std::max<int>(hi, r.hi);
	}
	
	auto it=std::upper_bound(code->lineStarts.begin(), code->lineStarts.end(), lo);
	int line=it - code->lineStarts.begin();
	std::stringstream ss;
	switch (type) {
	case rf::MsgType::error:
	  ss << "<span style=\"color: red\">error</span>: ";
	  break;
	case rf::MsgType::warning:
	  ss << "<span style=\"color: yellow\">warning</span>: ";
	default:
	  break;
	}
	ss << " " << escaped(message) << "<br>";
	int startOfLine = code->lineStarts[line-1]+1;
	int endOfLine = code->lineStarts[line];
	ss << "<span style=\"color: green\">" << escaped(code->code.substr(startOfLine,endOfLine-startOfLine))
	   << "</span><br>";
	if (ranges.size() == 0 && mainToken) {
	  ss << escaped(std::string(mainToken.start-startOfLine, ' '))
	     << "<span style=\"color: blue\">"
	     << "^"
	     << escaped(std::string(std::max<size_t>(mainToken.length, 1)-1, '~'))
	     << "</span>";
	} else {
		std::string i(endOfLine - startOfLine, ' ');
		for (auto r: ranges)
		  for (int x=std::max<int>(startOfLine, r.lo); x < std::min<int>(endOfLine, r.hi); ++x) 
		    i[x-startOfLine] = '~';
		if (mainToken)
		  i[std::max<int>(size_t(0), std::min<int>(mainToken.start + (mainToken.length-1) / 2 - startOfLine, endOfLine-startOfLine-1))] = '^';
		ss << "<span style=\"color: blue\">" << escaped(i) << "</span>";
	}
	interperter->doDisplay(QString::fromUtf8(ss.str().c_str()));
  }


  virtual void report(rf::MsgType type, std::string message) override {
    std::stringstream ss;
    switch (type) {
    case rf::MsgType::error:
      ss << "<span style=\"color: red\">error</span>: ";
      break;
    case rf::MsgType::warning:
      ss << "<span style=\"color: yellow\">warning</span>: ";
    default:
      break;
    }
    ss << " " << escaped(message);
    interperter->doDisplay(QString::fromUtf8(ss.str().c_str()));
  }

  virtual void print(Type type, std::string repr) override {
    std::stringstream ss;
    ss /*<< "<span style=\"color: green\">=&nbsp;</span>" */
       << escaped(repr);
    interperter->doDisplay(QString::fromUtf8(ss.str().c_str()));
  }

  virtual void saveRelation(rm_object * o, const char * name) override {}
  virtual rm_object * loadRelation(const char * name) override { return 0; }
  virtual bool hasRelation(const char * name) override { return false; }

private:
  Interpreter * interperter;
};


class InterpreterPrivate {

public:
  std::shared_ptr<rasmus::frontend::Interperter> interpreter;
  std::shared_ptr<rasmus::frontend::Callback> callback;

  InterpreterPrivate(Interpreter * interperter) {
    int options = 0; // dump no info on terminal.
    callback = std::make_shared<GuiCallBack>(interperter);
    interpreter=rasmus::frontend::makeInterperter(callback);
    interpreter->setup(options, std::string("Interpreter"));    
  }
};

Interpreter::Interpreter(QObject *parent) : QObject(parent) {
  d_ptr = new InterpreterPrivate(this);
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

void Interpreter::getCurrentEnvironment() {

}

void Interpreter::doDisplay(QString string) {
  emit display(string);
}

