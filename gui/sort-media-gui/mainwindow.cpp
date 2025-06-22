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

    QWidget* widget = new QWidget;
    widget->setLayout(layout);
    setCentralWidget(widget);

    QSettings settings;
    settings.beginGroup("sort_media_gui");
    sourceDir = settings.value("source_dir", "").toString();
    imageDir = settings.value("image_dir", "").toString();
    videoDir = settings.value("video_dir", "").toString();
    useModificationTimeAsCreated = settings.value("use_modification_time_as_created", false).toBool();

    sourceLabel->setText(sourceDir);
    imageLabel->setText(imageDir);
    videoLabel->setText(videoDir);
    useModificationTimeAsCreatedCheckBox->setChecked(useModificationTimeAsCreated);
}

MainWindow::~MainWindow() {
    QSettings settings;
    settings.beginGroup("sort_media_gui");
    settings.setValue("source_dir", sourceDir);
    settings.setValue("image_dir", imageDir);
    settings.setValue("video_dir", videoDir);
    settings.setValue("use_modification_time_as_created", useModificationTimeAsCreatedCheckBox->isChecked());
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
    dialog->resize(800, 600);
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
