#include "widget.h"
#include "ui_widget.h"

#include <QFileDialog>
#include <QDir>
#include <QStandardItem>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    setAcceptDrops(true);

    ui->lineListName->setPlaceholderText(QString("Write name of playlist"));

    Playlists = new QList<QList<QString>*>();
    Playlists->append(new QList<QString>());
    ui->menu_playlists->addItem("Default");

    // Настройка таблицы плейлиста
    m_playListModel = new QStandardItemModel(this);
    ui->playlistView->setModel(m_playListModel);    // Устанавливаем модель данных в TableView

    ui->playlistView->hideColumn(1);    // Скрываем колонку, в которой хранится путь к файлу
    ui->playlistView->horizontalHeader()->hide();
    ui->playlistView->verticalHeader()->setVisible(false);                  // Скрываем нумерацию строк
    ui->playlistView->setSelectionBehavior(QAbstractItemView::SelectRows);  // Включаем выделение строк
    ui->playlistView->setSelectionMode(QAbstractItemView::SingleSelection); // Разрешаем выделять только одну строку
    ui->playlistView->setEditTriggers(QAbstractItemView::NoEditTriggers);   // Отключаем редактирование
    // Включаем подгонку размера последней видимой колонки к ширине TableView
    ui->playlistView->horizontalHeader()->setStretchLastSection(true);

    m_player = new QMediaPlayer(this);          // Инициализируем плеер
    m_playlist = new QMediaPlaylist(m_player);  // Инициализируем плейлист
    m_player->setPlaylist(m_playlist);          // Устанавливаем плейлист в плеер
    m_player->setVolume(70);                    // Устанавливаем громкость воспроизведения треков
    m_playlist->setPlaybackMode(QMediaPlaylist::Loop);  // Устанавливаем циклический режим проигрывания плейлиста

    // подключаем кнопки управления к слотам управления
    // Здесь отметим, что навигация по плейлисту осуществляется именно через плейлист
    // а запуск/пауза/остановка через сам плеер
    connect(ui->btn_previous, &QToolButton::clicked, m_playlist, &QMediaPlaylist::previous);
    connect(ui->btn_next, &QToolButton::clicked, m_playlist, &QMediaPlaylist::next);
    connect(ui->btn_play, &QToolButton::clicked, m_player, &QMediaPlayer::play);
    connect(ui->btn_pause, &QToolButton::clicked, m_player, &QMediaPlayer::pause);
    connect(ui->btn_stop, &QToolButton::clicked, m_player, &QMediaPlayer::stop);
    connect(ui->upButton, &QToolButton::clicked, m_player, &QMediaPlayer::setPosition);
    connect(ui->downButton, &QToolButton::clicked, m_player, &QMediaPlayer::setPosition);

    connect(m_player, &QMediaPlayer::durationChanged, this, &Widget::on_durationSlider_valueChanged);
    connect(m_player, &QMediaPlayer::positionChanged, this, &Widget::on_positionSlider_valueChanged);

    // При даблклике по треку в таблице устанавливаем трек в плейлисте
    connect(ui->playlistView, &QTableView::doubleClicked, [this](const QModelIndex &index){
        m_playlist->setCurrentIndex(index.row());
    });

    // при изменении индекса текущего трека в плейлисте, устанавливаем название файла в специальном лейбле
    connect(m_playlist, &QMediaPlaylist::currentIndexChanged, [this](int index){
        ui->currentTrack->setText(m_playListModel->data(m_playListModel->index(index, 0)).toString());
    });
}

Widget::~Widget()
{
    delete ui;
    delete m_playListModel;
    delete m_playlist;
    delete m_player;
    delete Playlists;
}

void Widget::on_btn_add_clicked()
{
    // Если все плейлисты удалены
    if (ban_to_add == 1)
    {
        ui->currentTrack->setStyleSheet("QLabel { color : red; }");
        ui->currentTrack->setText("First of all create playlist!");
        return;
    }

    // С помощью диалога выбора файлов делаем множественный выбор mp3 файлов
    QStringList files = QFileDialog::getOpenFileNames(this,
                                                      tr("Open files"),
                                                      "D:/Projects/CHMI",
                                                      tr("Audio Files (*.mp3)"));

    // Далее устанавливаем данные по именам и пути к файлам↓
    // в плейлист и таблицу отображающую плейлист↑
    foreach (QString filePath, files)
    {
        QList<QStandardItem *> items;

        items.append(new QStandardItem(QDir(filePath).dirName()));
        //items.append(new QStandardItem(filePath));

        Playlists->at(curPlaylist)->append(filePath);
        m_playListModel->appendRow(items);
        m_playlist->addMedia(QUrl(filePath));
    }
}

void Widget::dragEnterEvent(QDragEnterEvent *event)
{
    // Обязательно необходимо допустить событие переноса данных в область окна приложения
    event->accept();
}

void Widget::dropEvent(QDropEvent *event)
{
    // Если все плейлисты удалены
    if (ban_to_add == 1)
    {
        ui->currentTrack->setStyleSheet("QLabel { color : red; }");
        ui->currentTrack->setText("First of all create playlist!");
        return;
    }

    QString filePath = event->mimeData()->urls()[0].toLocalFile();

    QList<QStandardItem *> items;

    items.append(new QStandardItem(QDir(filePath).dirName()));
    //items.append(new QStandardItem(filePath));

    Playlists->at(curPlaylist)->append(filePath);
    m_playListModel->appendRow(items);
    m_playlist->addMedia(QUrl(filePath));
}

void Widget::on_btn_delete_clicked()
{
    ui->currentTrack->clear();
    m_playListModel->removeRow(ui->playlistView->currentIndex().row(), QModelIndex());
}

void Widget::on_durationSlider_valueChanged(qint64 value)
{
    ui->durationSlider->setMaximum(value);
}

void Widget::on_positionSlider_valueChanged(qint64 value)
{
    ui->durationSlider->setValue(value);
}


void Widget::on_upButton_clicked()
{
    int selectedRow = ui->playlistView->currentIndex().row();
    if(m_playListModel->rowCount() < 2 || selectedRow == 0)
        return;

    QModelIndex prev_row = ui->playlistView->model()->index(selectedRow - 1, 0);

    QList<QStandardItem *> list = m_playListModel->takeRow(ui->playlistView->currentIndex().row());
    m_playListModel->insertRow(selectedRow - 1, list);
    ui->playlistView->setCurrentIndex(prev_row);
}

void Widget::on_downButton_clicked()
{
    int selectedRow = ui->playlistView->currentIndex().row();
    if(m_playListModel->rowCount() < 2 || selectedRow == m_playListModel->rowCount() - 1)
       return;

    QModelIndex next_row = ui->playlistView->model()->index(selectedRow + 1, 0);

    QList<QStandardItem *> list = m_playListModel->takeRow(ui->playlistView->currentIndex().row());
    m_playListModel->insertRow(selectedRow + 1, list);

    ui->playlistView->setCurrentIndex(next_row);
}

void Widget::on_lineListName_returnPressed()
{
    on_btn_create_playlist_clicked();
}

void Widget::on_btn_create_playlist_clicked()
{
    ui->currentTrack->clear();
    m_playListModel->clear();

    Playlists->append(new QList<QString>());

    QString nameList = ui->lineListName->text();
    if (nameList == "")
    {
        nameList += "Playlist ";
        nameList += QString::number(Playlists->count());
    }

    ui->menu_playlists->addItem(nameList);

    curPlaylist++;
    ban_to_add = 0;
}


void Widget::on_btn_delete_playlist_clicked()
{
    ui->menu_playlists->removeItem(ui->menu_playlists->currentIndex());
    ui->currentTrack->clear();
    m_playListModel->clear();
    m_playlist->clear();
    Playlists->removeAt(curPlaylist);

    if (Playlists->count() == 0)
    {
        ban_to_add = 1;
        return;
    }

    curPlaylist = Playlists->count() - 1;
    on_menu_playlists_activated(curPlaylist);
}

void Widget::on_menu_playlists_activated(int index)
{
    ui->currentTrack->clear();
    m_playListModel->clear();
    curPlaylist = index;

    for (int i = 0; i < Playlists->at(curPlaylist)->count(); i++)
    {
        m_playListModel->appendRow(new QStandardItem(QDir(Playlists->at(curPlaylist)->at(i)).dirName()));
        m_playlist->addMedia(QUrl(Playlists->at(curPlaylist)->at(i)));
    }
}


