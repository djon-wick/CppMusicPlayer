#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    // Метод события перетаскивания
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    // Метод события отпускания объекта с данными
    virtual void dropEvent(QDropEvent *event) override;

private slots:
    void on_btn_add_clicked();              // Слот для обработки добавления треков через диалоговое окно
    void on_btn_delete_clicked();

    void on_durationSlider_valueChanged(qint64 value);
    void on_positionSlider_valueChanged(qint64 value);

    void on_upButton_clicked();
    void on_downButton_clicked();

    void on_lineListName_returnPressed();

    void on_btn_create_playlist_clicked();
    void on_btn_delete_playlist_clicked();

    void on_menu_playlists_activated(int index);

private:
    Ui::Widget *ui;
    QStandardItemModel  *m_playListModel;   // Модель данных плейлиста для отображения
    QMediaPlayer        *m_player;          // Проигрыватель треков
    QMediaPlaylist      *m_playlist;        // Плейлиста проигрывателя
    QList<QList<QString> *> *Playlists;
    int curPlaylist = 0;
    int ban_to_add = 0;
};

#endif // WIDGET_H
