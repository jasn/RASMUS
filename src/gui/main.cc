#include <QApplication>
#include <QTextEdit>
#include "ui_main.h"
#include <interpreter.hh>


class MainWindow : public QMainWindow {

  Q_OBJECT

public:
  
  Ui::MainWindow ui;

  Interpreter *interpreter;

  MainWindow() {
    ui.setupUi(this);
    interpreter = new Interpreter(this);
    
    QObject::connect(ui.console, SIGNAL(run(QString)), interpreter, SLOT(run(QString)));
    QObject::connect(interpreter, SIGNAL(incomplete()), ui.console, SLOT(incomplete()));
    QObject::connect(interpreter, SIGNAL(complete()), ui.console, SLOT(complete()));
    QObject::connect(interpreter, SIGNAL(display(QString)), ui.console, SLOT(display(QString)));
    ui.console->complete();
  }

};

int main(int argc, char * argv[]) {
  
  QApplication app(argc, argv);

  MainWindow window;

  window.show();
  
  app.exec();
  
    
}

#include <main.moc>
