
#include <QPainter>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QFocusEvent>
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/ui/ui_utils.h"
#include "onyx/ui/content_view.h"

namespace ui
{

static QPoint s_mouse;
static const int MARGIN = 4;

ContentView::ContentView(QWidget *parent)
        : QWidget(parent)
        , data_(0)
        , pressed_(false)
        , checked_(false)
        , repaint_on_mouse_release_(true)
        , pen_width_(3)
        , bk_color_(Qt::white)
{
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(false);
}

ContentView::~ContentView()
{
}

void ContentView::setChecked(bool checked)
{
    checked_ = checked;
}

bool ContentView::isChecked()
{
    return checked_;
}

bool ContentView::updateData(OData* data, bool force)
{
    if (data_ == data && !force)
    {
        return false;
    }
    data_ = data;
    updateView();
    update();
    return true;
}

OData * ContentView::data()
{
    return data_;
}

bool ContentView::isPressed()
{
    return pressed_;
}

void ContentView::setPressed(bool p)
{
    pressed_ = p;
}

void ContentView::activate(int user_data)
{
    if (data())
    {
        emit activated(this, user_data);
    }
}

void ContentView::repaintAndRefreshScreen()
{
    update();
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW);
}

void ContentView::setRepaintOnMouseRelease(bool enable)
{
    repaint_on_mouse_release_ = enable;
}

void ContentView::mousePressEvent(QMouseEvent *event)
{
    s_mouse = event->globalPos();
    if (data())
    {
        setPressed(true);
        repaintAndRefreshScreen();
    }
    QWidget::mousePressEvent(event);
}

void ContentView::mouseReleaseEvent(QMouseEvent *event)
{
    bool broadcast = false;
    if (isPressed())
    {
        broadcast = true;
    }
    else
    {
        emit mouse(s_mouse, event->globalPos());
    }
    setPressed(false);
    if (data() && repaint_on_mouse_release_)
    {
        repaintAndRefreshScreen();
    }
    QWidget::mouseReleaseEvent(event);
    if (broadcast)
    {
        activate();
    }
}

void ContentView::mouseMoveEvent(QMouseEvent * e)
{
    if (isPressed() && !rect().contains(e->pos()) && data())
    {
        setPressed(false);
        repaintAndRefreshScreen();
    }
}

void ContentView::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return)
    {
        activate();
        e->accept();
        return;
    }
    e->ignore();
    emit keyRelease(this, e);
}

void ContentView::changeEvent(QEvent *event)
{
}

void ContentView::resizeEvent(QResizeEvent * event)
{
    QWidget::resizeEvent(event);
}

bool ContentView::event(QEvent * e)
{
    switch (e->type())
    {
    case QEvent::HoverMove:
    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
        e->accept();
        return true;
    default:
        break;
    }
    return QWidget::event(e);
}

void ContentView::focusInEvent(QFocusEvent * e)
{
    QWidget::focusInEvent(e);
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW);
}

void ContentView::focusOutEvent(QFocusEvent * e)
{
    QWidget::focusOutEvent(e);
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW);
}

void ContentView::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.fillRect(rect(), bkColor());

    if (data())
    {
        if (isPressed())
        {
            //painter.fillRect(rect(), Qt::darkGray);
        }
        if (hasFocus())
        {
            QPen pen;
            pen.setWidth(penWidth());
            painter.setPen(pen);
            painter.drawRoundedRect(rect().adjusted(0, 0, -penWidth() , -penWidth()), 5, 5);
        }
    }
}


CoverView::CoverView(QWidget *parent)
: ContentView(parent)
{
}

CoverView::~CoverView()
{
}

const QString CoverView::type()
{
    return "CoverView";
}

void CoverView::updateView()
{
    update();
}

void CoverView::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.fillRect(rect(), bkColor());

    if (data())
    {
        if (isPressed() || isChecked())
        {
            painter.fillRect(rect().adjusted(penWidth(), penWidth(), -penWidth() - 1, -penWidth() - 1), Qt::gray);
        }
        if (hasFocus())
        {
            QPen pen;
            pen.setWidth(penWidth());
            painter.setPen(pen);
            painter.drawRoundedRect(rect().adjusted(0, 0, -penWidth() , -penWidth()), 5, 5);
        }

        drawCover(painter, rect());
        if (isPressed() || isChecked())
        {
            painter.setPen(Qt::white);
        }
        drawTitle(painter, rect());
    }
}

void CoverView::drawCover(QPainter & painter, QRect rect)
{
    if (data() && data()->contains("cover"))
    {
        QPixmap pixmap(qVariantValue<QPixmap>(data()->value("cover")));
        int x = (rect.width() - pixmap.width()) / 2;
        painter.drawPixmap(x, MARGIN, pixmap);
    }
}

void CoverView::drawTitle(QPainter & painter, QRect rect)
{
    if (data() && data()->contains("title"))
    {
        QFont font;
        font.setPointSize(ui::defaultFontPointSize());
        painter.setFont(font);
        painter.drawText(rect, Qt::AlignCenter, data()->value("title").toString());
    }
}


CheckBoxView::CheckBoxView(QWidget *parent)
: ContentView(parent)
{

}

CheckBoxView::~CheckBoxView()
{
}

const QString CheckBoxView::type()
{
    return "CheckBoxView";
}

void CheckBoxView::updateView()
{
    update();
}

void CheckBoxView::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.fillRect(rect(), bkColor());

    if (data())
    {
        static const QString CHECKED = "checked";
        if (data()->contains(CHECKED))
        {
            setChecked(qVariantValue<bool> (data()->value(CHECKED)));
        }
        if (isPressed() || isChecked())
        {
            painter.fillRect(rect().adjusted(penWidth(), penWidth(), -penWidth() - 1, -penWidth() - 1), Qt::gray);
        }
        if (hasFocus())
        {
            QPen pen;
            pen.setWidth(penWidth());
            painter.setPen(pen);
            painter.drawRoundedRect(rect().adjusted(0, 0, -penWidth() , -penWidth()), 5, 5);
        }

        QRect check_box_r = drawCheckBox(painter, rect());

        int icon_x = check_box_r.right() + MARGIN;
        QRect icon_r = drawCover(painter, QRect(icon_x, rect().y(),
                rect().width()-icon_x, rect().height()));

        if (isPressed() || isChecked())
        {
            painter.setPen(Qt::white);
        }
        int title_x = icon_r.right() + MARGIN;
        drawTitle(painter, QRect(title_x, rect().y(),
                rect().width()-title_x, rect().height()));
    }
}

QRect CheckBoxView::drawCheckBox(QPainter & painter, QRect rect)
{
    int width = checkBoxViewWidth();
    int height = width;
    int x = rect.x() + 20;
    int y = (rect.height() - height)/2;
    QRect check_box_rect(x, y, width, height);
    if (isChecked())
    {
        painter.setPen(QPen(Qt::white, 2));
        painter.fillRect(check_box_rect, Qt::black);
    }
    else
    {
        painter.setPen(QPen(Qt::black, 2));
        painter.fillRect(check_box_rect, Qt::white);
    }
    painter.drawRect(check_box_rect);
    return check_box_rect;
}

QRect CheckBoxView::drawCover(QPainter & painter, QRect rect)
{
    QRect icon_rect(rect.topLeft(), rect.topLeft());
    if (data() && data()->contains("cover"))
    {
        QPixmap pixmap(qVariantValue<QPixmap>(data()->value("cover")));
        painter.drawPixmap(MARGIN, (rect.height() - pixmap.height()) / 2, pixmap);
        icon_rect.setRight(pixmap.width());
    }
    return icon_rect;
}

void CheckBoxView::drawTitle(QPainter & painter, QRect rect)
{
    if (data() && data()->contains("title"))
    {
        rect.adjust(MARGIN, 0, 0, 0);
        QFont font;
        font.setPointSize(ui::defaultFontPointSize());
        painter.setFont(font);
        painter.drawText(rect, Qt::AlignLeft|Qt::AlignVCenter,
                data()->value("title").toString());
    }
}




LineEditView::LineEditView(QWidget *parent)
    : ContentView(parent)
    , inner_edit_(this)
    , layout_(this)
    , forward_focus_(true)
{
    createLayout();
}

LineEditView::~LineEditView()
{
}

const QString LineEditView::type()
{
    return "LineEditView";
}

void LineEditView::createLayout()
{
    layout_.setContentsMargins(MARGIN, 0, MARGIN, 0);
    layout_.addWidget(&inner_edit_, 1);
    connect(&inner_edit_, SIGNAL(outOfRange(QKeyEvent*)), this, SLOT(onEditOutOfRange(QKeyEvent*)));
}

void LineEditView::updateView()
{
    if (data())
    {
        if (data()->contains("title"))
        {
            QString text = data()->value("title").toString();
            inner_edit_.setText(text);
        }
    }
}

void LineEditView::focusInEvent(QFocusEvent * event)
{
    if (forward_focus_)
    {
        inner_edit_.setFocus();
        onyx::screen::watcher().enqueue(&inner_edit_,
                onyx::screen::ScreenProxy::DW);
    }
}

void LineEditView::focusOutEvent(QFocusEvent * event)
{
    forward_focus_ = true;
}

void LineEditView::onEditOutOfRange(QKeyEvent *ke)
{
    forward_focus_ = false;
    setFocus();
    emit keyRelease(this, ke);
    onyx::screen::watcher().enqueue(&inner_edit_, onyx::screen::ScreenProxy::DW);
}

void LineEditView::keyPressEvent(QKeyEvent * src)
{
    QKeyEvent * key = new QKeyEvent(src->type(), src->key(), src->modifiers(), src->text());
    QApplication::sendEvent(&inner_edit_, key);
}

void LineEditView::keyReleaseEvent(QKeyEvent * event)
{
    if (Qt::Key_Escape == event->key())
    {
        event->ignore();
        return;
    }
    event->accept();
}


ClockView::ClockView(QWidget *parent)
: ContentView(parent)
{
}

ClockView::~ClockView()
{
}

const QString ClockView::type()
{
    return "ClockView";
}

void ClockView::updateView()
{
}

void ClockView::paintEvent(QPaintEvent * event)
{
    static const QPoint hourHand[3] = {
         QPoint(3, 4),
         QPoint(-4, 4),
         QPoint(0, -40)
     };
     static const QPoint minuteHand[3] = {
         QPoint(3, 4),
         QPoint(-3, 4),
         QPoint(0, -70)
     };

     QColor hourColor(Qt::black);
     QColor minuteColor(Qt::black);

     int side = qMin(width(), height());
     QTime time = QTime::currentTime();

     QPainter painter(this);
     painter.fillRect(rect(), bkColor());
     painter.setRenderHint(QPainter::Antialiasing);
     painter.translate(width() / 2, height() / 2);
     painter.scale(side / 200.0, side / 200.0);

     painter.setPen(Qt::NoPen);
     painter.setBrush(hourColor);

     painter.save();
     painter.rotate(30.0 * ((time.hour() + time.minute() / 60.0)));
     painter.drawConvexPolygon(hourHand, 3);
     painter.restore();

     painter.setPen(hourColor);

     for (int i = 0; i < 12; ++i) {
         painter.drawLine(88, 0, 98, 0);
         painter.rotate(30.0);
     }

     painter.setPen(Qt::NoPen);
     painter.setBrush(minuteColor);

     painter.save();
     painter.rotate(6.0 * (time.minute() + time.second() / 60.0));
     painter.drawConvexPolygon(minuteHand, 3);
     painter.restore();

     painter.setPen(minuteColor);

     for (int j = 0; j < 60; ++j) {
         if ((j % 5) != 0)
             painter.drawLine(92, 0, 96, 0);
         painter.rotate(6.0);
     }
}

}

