#ifndef TAGTEXTEDIT_H
#define TAGTEXTEDIT_H

#include <QObject>
#include <QCompleter>
#include <QScrollBar>

#include "searchdialog.h"

class TagTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    TagTextEdit(QCompleter *completer, QWidget *parent = nullptr) : QPlainTextEdit(parent), completer(completer) {}
    virtual ~TagTextEdit() override {}
private:
    QCompleter *completer;
    QString textUnderCursor() const
    {
        QTextCursor cursor = static_cast<QPlainTextEdit*>(completer->widget())->textCursor();
        cursor.select(QTextCursor::WordUnderCursor);
        return cursor.selectedText();
    }
protected:
    virtual void keyPressEvent(QKeyEvent *event) override
    {
        QPlainTextEdit::keyPressEvent(event);
        if (event->type() == QEvent::KeyPress)
        {
            if (completer->popup()->isVisible())
            {
                completer->popup()->hide();
            }
            QString completionPrefix = textUnderCursor();
            QString oldPrefix = completer->completionPrefix();
            if (completionPrefix != oldPrefix)
            {
                completer->setCompletionPrefix(completionPrefix);
                completer->popup()->setCurrentIndex(completer->completionModel()->index(0, 0));
            }
            QRect rect =  static_cast<QPlainTextEdit*>(completer->widget())->cursorRect();
            rect.setWidth(completer->popup()->sizeHintForColumn(0) + completer->popup()->verticalScrollBar()->sizeHint().width());
            completer->complete(rect);
        }
    }
};
#endif // TAGTEXTEDIT_H
