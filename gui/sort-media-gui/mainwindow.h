#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QPushButton* selectSourceDirectory;
    QPushButton* selectImageDestination;
    QPushButton* selectVideoDestination;
    QPushButton* parseButton;
    QCheckBox* useModificationTimeAsCreatedCheckBox;

    QLabel* sourceLabel;
    QLabel* imageLabel;
    QLabel* videoLabel;

    QString sourceDir;
    QString imageDir;
    QString videoDir;
    bool useModificationTimeAsCreated;


private slots:
    void slot_selectSource();
    void slot_selectImageDestination();
    void slot_selectVideoDestination();
    void slot_parse();
};
#endif // MAINWINDOW_H
