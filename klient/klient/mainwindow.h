#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "text.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    CLIENT *clnt;
    Data  *result_1;
    Data  binaryzuj_1_arg;

    void otworz_plik();
    void binaryzuj(QImage *image, int prog);
    
private slots:
    void on_pushButton_clicked();

    void on_pushButton_3_clicked();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
