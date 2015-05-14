#ifndef mqMesh_H
#define mqMesh_H

#include <QMainWindow>
#include <QTextStream>
#include <QThread>
#include <QMessageBox>

namespace Ui {
class mqMesh;
}



//======================================
// QMainWindow -> mqMesh, class declaration
//======================================

class mqMesh : public QMainWindow
{
    Q_OBJECT

public:
    explicit mqMesh(QWidget *parent = 0);
    ~mqMesh();

public slots:
    void s_displayTextString(QString);
    
    void slot_displayMessageBox(QString);

private slots:
    void on_quitButton_clicked();

    void on_actionOpen_triggered();

    void on_actionStart_triggered();

    void on_actionStop_triggered();

    void on_actionFontSelection_triggered();

    void on_actionPrint_triggered();

signals:
    void sg_startComputation();
    void sg_stopComputation();

private:
    Ui::mqMesh *ui;
    QThread numerical_computation_thread;
    QString edittext_string;
    QMessageBox Msgbox;
};

//======================================
// QObject -> mqNumericalComputation, class declaration
//======================================

class mqNumericalComputation : public QObject
{
     Q_OBJECT
     
 

 public slots:
     void s_startComputation();

     void s_stopComputation();

 signals:
     void sg_displayTextString(QString);
     void signal_displayMessageBox(QString);
     void sg_finished();
};

#endif // mqMesh_H
