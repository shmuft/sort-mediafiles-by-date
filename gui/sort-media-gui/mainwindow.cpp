#include "mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QProcess>
#include <QDebug>
#include <QSettings>
#include <QTextDocument>
#include <QTextEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    selectSourceDirectory = new QPushButton(tr("Выбрать директорию для рассортировки"));
    connect(selectSourceDirectory, &QPushButton::clicked, this, &MainWindow::slot_selectSource);

    selectImageDestination = new QPushButton(tr("Выбрать куда складывать изображения"));
    connect(selectImageDestination, &QPushButton::clicked, this, &MainWindow::slot_selectImageDestination);

    selectVideoDestination = new QPushButton(tr("Выбрать куда складывать видео"));
    connect(selectVideoDestination, &QPushButton::clicked, this, &MainWindow::slot_selectVideoDestination);

    parseButton = new QPushButton(tr("Распарсить файлы!"));
    connect(parseButton, &QPushButton::clicked, this, &MainWindow::slot_parse);

    sourceLabel = new QLabel;
    imageLabel = new QLabel;
    videoLabel = new QLabel;

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(selectSourceDirectory);
    layout->addWidget(selectImageDestination);
    layout->addWidget(selectVideoDestination);
    layout->addWidget(parseButton);
    layout->addWidget(sourceLabel);
    layout->addWidget(imageLabel);
    layout->addWidget(videoLabel);
    QWidget* widget = new QWidget;
    widget->setLayout(layout);
    setCentralWidget(widget);

    QSettings settings;
    settings.beginGroup("sort_media_gui");
    sourceDir = settings.value("source_dir", "").toString();
    imageDir = settings.value("image_dir", "").toString();
    videoDir = settings.value("video_dir", "").toString();

    sourceLabel->setText(sourceDir);
    imageLabel->setText(imageDir);
    videoLabel->setText(videoDir);
}

MainWindow::~MainWindow() {
    QSettings settings;
    settings.beginGroup("sort_media_gui");
    settings.setValue("source_dir", sourceDir);
    settings.setValue("image_dir", imageDir);
    settings.setValue("video_dir", videoDir);
}

void MainWindow::slot_selectSource()
{
    sourceDir = QFileDialog::getExistingDirectory(this, "Выбор папки для рассортировки", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
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

    QTextEdit *doc = new QTextEdit;

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(doc);

    QDialog* dialog = new QDialog(this);
    dialog->setLayout(layout);
    dialog->show();


    QString program = "sort-media.exe";
    QStringList arguments;
    arguments.append(QString("--source_dir=%1").arg(sourceDir));
    arguments.append(QString("--export_dir=%1").arg(imageDir));
    arguments.append(QString("--video_export_dir=%1").arg(videoDir));


    QProcess myProcess(this);
    connect(&myProcess, &QProcess::readyReadStandardOutput, doc,
            [&myProcess, doc]()
            {
        doc->moveCursor(QTextCursor::End);
        doc->insertPlainText(myProcess.readAllStandardOutput());
    });

    myProcess.start(program, arguments);
    if (!myProcess.waitForStarted())
        return;

    if (!myProcess.waitForFinished())
        return;

    QByteArray result = myProcess.readAll();
    qDebug() << result;
}
