#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "text.h"
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include "qtextcodec.h"
#include <QColor>
#include <QRgb>
#include <QMessageBox>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));       //
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8")); //
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));   //polskie znaki w polach tekstowych

    ui->setupUi(this);
    ui->lineEdit->setValidator(new QIntValidator(0, 255, this)); //ograniczenie pola tekstowego, wartości 0-255
    ui->lineEdit->setText("128"); //domyślna wartość (dla ułatwienia sprawdzania działania)

    char host[] = "localhost";
    clnt = clnt_create(host, TEXT_PROG, TEXT_VERS, "tcp");
    if(clnt == NULL) {
        clnt_pcreateerror(host);
        exit(1);
    }
}

QImage *image = new QImage(); //wymagane do działania na kliencie

MainWindow::~MainWindow()
{
    clnt_destroy(clnt);
    delete ui;
}

void MainWindow::on_pushButton_clicked() {
    if(ui->lineEdit->text().isEmpty()) {
        ui->label_4->setText("Nie podano progu");
    }
    else {
        if(!image->isNull()) {
            QImage *nowy = new QImage(*image); //kopia obrazka żeby było możliwe wielokrotne binaryzowanie z różnymi programi
            binaryzuj(nowy, ui->lineEdit->text().toInt());
        }
        else {
            ui->label_2->setText("Obrazek po binaryzacji");
        }
    }
}

void MainWindow::otworz_plik() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Otwórz..."), QDir::currentPath()+"/obrazy_testowe", tr("Pliki graficzne (*.jpeg *.jpg *.bmp *.png *.gif)"));

    image = new QImage(filename);

    if(!image->isNull()) {
        if((image->width()>1024) || (image->height()>1024)) {
            QMessageBox msgBox;
            msgBox.setText("Obrazek jest za duży. Maksymalny rozmiar obrazka: 1024x1024");
            msgBox.setInformativeText("Czy chcesz wczytać inny obrazek?");
            msgBox.setStandardButtons(QMessageBox::Open | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            int ret = msgBox.exec();

            switch(ret) {
                case QMessageBox::Open:
                    otworz_plik();
                    break;
            }
        }
        else {
            QImage alpha = image->alphaChannel(); //zapisywanie kanału alfa do obrazka w skali szarości
            image->setAlphaChannel(alpha);        //wymagane do działania dla niektórych obrazków
            for(int y=0; y<image->height(); y++) {
                for(int x=0; x<image->width(); x++) {
                    QRgb color = image->pixel(x,y); //pobranie koloru piksela
                    unsigned char gray = (qRed(color) + qGreen(color) + qBlue(color))/3; //zamiana na odcienie szarości
                    image->setPixel(x,y, qRgb(gray, gray, gray)); //ustawienie koloru w odcieniu szarości
                }
            }
            ui->label->setPixmap(QPixmap::fromImage(*image)); //narysowanie obrazka w polu
        }
    }
    else {
        ui->label->setText("Nie wczytano obrazka");
    }
}

void MainWindow::on_pushButton_3_clicked() {
    otworz_plik();
}

void MainWindow::binaryzuj(QImage *image1, int prog) {

    uchar nowa[1048576]; //nowa tablica do której będzie zapisana zawartość obrazka
    for(int i=0; i<1048576; i++) {
        nowa[i] = 128;
        binaryzuj_1_arg.Buffer[i] = 128;
    }

    for(int y=0; y<image1->height(); y++) {
        for(int x=0; x<image1->width(); x++) {
            QRgb color = image1->pixel(x,y); //pobranie koloru piksela
            unsigned char gray = (qRed(color) + qGreen(color) + qBlue(color))/3; //zamiana na odcienie szarości
            nowa[y*image1->width() + x] = gray; //wpisanie do tablicy koloru obrazka
        }
    }

    ui->label->setPixmap(QPixmap::fromImage(*image1));

    //---PRZYGOTOWANIE DO WYSŁANIA NA SERWER---
    for(int i=0; i<1048576; i++) {
        binaryzuj_1_arg.Buffer[i] = nowa[i];
    }

    binaryzuj_1_arg.prog = (unsigned char)prog;

    result_1 = binaryzuj_1(&binaryzuj_1_arg, clnt); //wywołanie funkcji na serwerze

    //---OBSŁUGA REZULATU DZIAŁANIA SERWERA---
    if (result_1 == (Data *) NULL) {
        clnt_perror (clnt, "call failed");
    }
    else {
        for(int y=0; y<image1->height(); y++) {
            for(int x = 0; x<image1->width(); x++) {
                unsigned char kolor = result_1->Buffer[y*image1->width() + x]; //pobranie koloru piksela
                image1->setPixel(x,y, qRgb(kolor, kolor, kolor)); //ustawienie koloru piksela
            }
        }

        ui->label_2->setPixmap(QPixmap::fromImage(*image1));  //narysowanie obrazka w polu
        ui->label_5->setText((QString().sprintf("  : %d", prog))); //wypisanie progu binaryzacji
    }
}

