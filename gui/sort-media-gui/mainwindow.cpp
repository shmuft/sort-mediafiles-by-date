#include "mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QProcess>
#include <QDebug>
#include <QSettings>
#include <QTextDocument>
#include <QTextEdit>
#include <QCoreApplication>
#include <QTimer>
#include <QListView>
#include <QStringListModel>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    selectSourceDirectory = new QPushButton(tr("Выбрать директорию для рассортировки"));
    connect(selectSourceDirectory, &QPushButton::clicked, this, &MainWindow::slot_selectSource);

    selectImageDestination = new QPushButton(tr("Выбрать куда складывать изображения"));
    connect(selectImageDestination, &QPushButton::clicked, this, &MainWindow::slot_selectImageDestination);

    selectVideoDestination = new QPushButton(tr("Выбрать куда складывать видео"));
    connect(selectVideoDestination, &QPushButton::clicked, this, &MainWindow::slot_selectVideoDestination);

    parseButton = new QPushButton(tr("Рассотировать!"));
    connect(parseButton, &QPushButton::clicked, this, &MainWindow::slot_parse);

    useModificationTimeAsCreatedCheckBox = new QCheckBox("Если нет даты в exif и в имени файла - использовать дату модификации файла");

    sourceLabel = new QLabel;
    imageLabel = new QLabel;
    videoLabel = new QLabel;

    QGridLayout *labelsLayout = new QGridLayout;
    labelsLayout->addWidget(new QLabel(tr("Откуда:")), 0, 0);
    labelsLayout->addWidget(sourceLabel, 0, 1);
    labelsLayout->addWidget(new QLabel(tr("Куда Фото:")), 1, 0);
    labelsLayout->addWidget(imageLabel, 1, 1);
    labelsLayout->addWidget(new QLabel(tr("Куда Видео:")), 2, 0);
    labelsLayout->addWidget(videoLabel, 2, 1);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(selectSourceDirectory);
    layout->addWidget(selectImageDestination);
    layout->addWidget(selectVideoDestination);
    layout->addWidget(useModificationTimeAsCreatedCheckBox);
    layout->addWidget(parseButton);
    layout->addLayout(labelsLayout);

    settingsView = new QListView;
    connect(settingsView, &QListView::clicked, this, &MainWindow::slot_settingsClicked);
    loadProfiles();
    settingsView->setModel(model);

    userName = new QLineEdit;

    QPushButton *addUserButton = new QPushButton(tr("Добавить"));
    QPushButton *deleteUserButton = new QPushButton(tr("Удалить"));

    connect(addUserButton, &QPushButton::clicked, this, &MainWindow::slot_addUser);
    connect(deleteUserButton, &QPushButton::clicked, this, &MainWindow::slot_deleteUser);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(settingsView);
    rightLayout->addStretch(1);
    rightLayout->addWidget(new QLabel(tr("Имя пользователя")));
    rightLayout->addWidget(userName);
    rightLayout->addWidget(addUserButton);
    rightLayout->addWidget(deleteUserButton);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addLayout(rightLayout);

    QWidget* widget = new QWidget;
    widget->setLayout(mainLayout);
    setCentralWidget(widget);


    if (!profileList.isEmpty()) {
        currentProfile = profileList.first();
        loadProfile(currentProfile);
        userName->setText(currentProfile);
    }

    QSettings settings;
    settings.beginGroup("sort_media_gui");
    useModificationTimeAsCreated = settings.value("use_modification_time_as_created", false).toBool();
    useModificationTimeAsCreatedCheckBox->setChecked(useModificationTimeAsCreated);
}

MainWindow::~MainWindow() {
    saveCurrentProfile();

    QSettings settings;
    settings.beginGroup("sort_media_gui");
    settings.setValue("use_modification_time_as_created", useModificationTimeAsCreatedCheckBox->isChecked());
}

void MainWindow::loadProfiles()
{
    QSettings settings;
    settings.beginGroup("sort_media_gui");

    profileList = settings.value("profile_list", QStringList{}).toStringList();
    model = new QStringListModel(profileList);
}

void MainWindow::saveCurrentProfile()
{
    QString profileName = userName->text().trimmed();
    if (profileName.isEmpty()) {
        if (!currentProfile.isEmpty()) {
            profileName = currentProfile;
        } else {
            return;
        }
    }

    QSettings settings;
    settings.beginGroup("sort_media_gui");
    settings.beginGroup("profiles");
    settings.setValue(profileName + "/source_dir", sourceDir);
    settings.setValue(profileName + "/image_dir", imageDir);
    settings.setValue(profileName + "/video_dir", videoDir);
    settings.endGroup();
}

void MainWindow::loadProfile(const QString &profileName)
{
    QSettings settings;
    settings.beginGroup("sort_media_gui");
    settings.beginGroup("profiles");

    sourceDir = settings.value(profileName + "/source_dir", "").toString();
    imageDir = settings.value(profileName + "/image_dir", "").toString();
    videoDir = settings.value(profileName + "/video_dir", "").toString();

    settings.endGroup();

    sourceLabel->setText(sourceDir);
    imageLabel->setText(imageDir);
    videoLabel->setText(videoDir);
}

void MainWindow::slot_settingsClicked(const QModelIndex &index)
{
    QString profileName = model->data(index).toString();

    saveCurrentProfile();

    currentProfile = profileName;
    userName->setText(profileName);
    loadProfile(profileName);
}

void MainWindow::slot_selectSource()
{
    sourceDir = QFileDialog::getExistingDirectory(this, "Выбор папки для рассортировки", sourceLabel->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (sourceDir.isEmpty())
    {
        QMessageBox::warning(this, tr("Внимание!"), tr("Не выбрана директория"));
        return;
    }
    sourceLabel->setText(sourceDir);
    saveCurrentProfile();
}

void MainWindow::slot_selectImageDestination()
{
    imageDir = QFileDialog::getExistingDirectory(this, "Выбор папки для изображений", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (imageDir.isEmpty())
    {
        QMessageBox::warning(this, tr("Внимание!"), tr("Не выбрана директория"));
        return;
    }
    imageLabel->setText(imageDir);
    saveCurrentProfile();
}

void MainWindow::slot_selectVideoDestination()
{
    videoDir = QFileDialog::getExistingDirectory(this, "Выбор папки для видео", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (videoDir.isEmpty())
    {
        QMessageBox::warning(this, tr("Внимание!"), tr("Не выбрана директория"));
        return;
    }
    videoLabel->setText(videoDir);
    saveCurrentProfile();
}

void MainWindow::slot_addUser()
{
    QString name = userName->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("Внимание!"), tr("Введите имя пользователя в поле 'Имя пользователя'"));
        return;
    }
    if (profileList.contains(name)) {
        QMessageBox::warning(this, tr("Внимание!"), tr("Пользователь '%1' уже существует").arg(name));
        return;
    }

    saveCurrentProfile();

    profileList.append(name);
    model->setStringList(profileList);

    currentProfile = name;
    loadProfile(name);

    QSettings settings;
    settings.beginGroup("sort_media_gui");
    settings.setValue("profile_list", profileList);
}

void MainWindow::slot_deleteUser()
{
    if (currentProfile.isEmpty()) {
        QMessageBox::warning(this, tr("Внимание!"), tr("Не выбран пользователь для удаления"));
        return;
    }

    QString msg = tr("Удалить пользователя '%1'?\nВсе данные этого пользователя будут удалены.")
                      .arg(currentProfile);
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Подтверждение удаления"), msg,
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    saveCurrentProfile();

    QSettings settings;
    settings.beginGroup("sort_media_gui");
    settings.beginGroup("profiles");
    settings.remove(currentProfile);
    settings.endGroup();

    profileList.removeAll(currentProfile);
    model->setStringList(profileList);

    if (!profileList.isEmpty()) {
        currentProfile = profileList.first();
        loadProfile(currentProfile);
        userName->setText(currentProfile);
    } else {
        currentProfile.clear();
        sourceDir.clear();
        imageDir.clear();
        videoDir.clear();
        sourceLabel->clear();
        imageLabel->clear();
        videoLabel->clear();
        userName->clear();
    }

    settings.setValue("profile_list", profileList);
}

void MainWindow::slot_parse()
{
    if (sourceDir.isEmpty()
        || imageDir.isEmpty()
        || videoDir.isEmpty())
    {
        QMessageBox::warning(this, tr("Внимание!"), tr("Выберите директории!"));
        return;
    }

    setEnabled(false);

    QTextEdit *doc = new QTextEdit;
    QLabel* label = new QLabel;
    QPushButton* closeButton = new QPushButton(tr("Закрыть"));
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(doc);
    layout->addWidget(closeButton);

    QDialog* dialog = new QDialog(this);
    connect(closeButton, &QPushButton::pressed, dialog, &QDialog::accept);

    dialog->setEnabled(false);
    dialog->setWindowModality(Qt::WindowModality::WindowModal);
    dialog->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    dialog->resize(1200, 600);
    dialog->setLayout(layout);

    dialog->show();

    QString program = "sort-media.exe";
    QStringList arguments;
    arguments.append(QString("--source_dir=%1").arg(sourceDir));
    arguments.append(QString("--export_dir=%1").arg(imageDir));
    arguments.append(QString("--video_export_dir=%1").arg(videoDir));
    if (useModificationTimeAsCreatedCheckBox->isChecked())
        arguments.append(QString("--use_mod_time_as_created"));
    arguments.append(QString("--sync_std_in_out"));

    QProcess myProcess(this);
    connect(&myProcess, &QProcess::readyReadStandardOutput, doc,
            [&myProcess, doc]()
            {
        QString str = myProcess.readAllStandardOutput();
        doc->moveCursor(QTextCursor::End);
        doc->insertPlainText(str);
        QCoreApplication::processEvents();
        myProcess.write("done\n");
    });

    myProcess.start(program, arguments);
    if (!myProcess.waitForStarted())
        return;

    if (!myProcess.waitForFinished(-1))
        return;

    label->setText(tr("Всё!"));
    doc->moveCursor(QTextCursor::End);
    dialog->setEnabled(true);
    setEnabled(true);
}
