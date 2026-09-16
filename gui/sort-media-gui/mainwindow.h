#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QStringList>
#include <QModelIndex>

class QStringListModel;
class QLineEdit;
class QListView;
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
    
    QStringListModel *model;
    QLineEdit *userName;
    QListView *settingsView;
    QString currentProfile;
    QStringList profileList;

    void loadProfiles();
    void saveCurrentProfile();
    void loadProfile(const QString &profileName);
    void slot_settingsClicked(const QModelIndex &index);
    void slot_selectSource();
    void slot_selectImageDestination();
    void slot_selectVideoDestination();
    void slot_parse();
    void slot_addUser();
    void slot_deleteUser();
};
#endif // MAINWINDOW_H
