// Help > Find Command; see CommandFinder.h.
#include "ui/CommandFinder.h"

#include <QAction>
#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QTimer>
#include <QVBoxLayout>

namespace qb {

static QString plain(QString t)
{
    t.replace(QLatin1String("&&"), QLatin1String("\x01"));
    t.remove(QLatin1Char('&'));
    t.replace(QLatin1Char('\x01'), QLatin1Char('&'));
    t.remove(QChar(0x2026));
    while (t.endsWith(QLatin1Char('.')))
        t.chop(1);
    return t.simplified();
}

static void walk(QMenu *menu, const QString &path, bool parentsOn, QList<CommandFinder::Entry> *out, QList<QMenu *> *seen)
{
    if (seen->contains(menu))
        return;
    seen->append(menu);
    for (QAction *ac : menu->actions()) {
        if (ac->isSeparator() || !ac->isVisible())
            continue;
        if (ac->objectName() == QLatin1String("qbWindowEntry"))
            continue;
        if (QMenu *sub = ac->menu()) {
            walk(sub, path + QLatin1String(" > ") + plain(sub->title().isEmpty() ? ac->text() : sub->title()),
                 parentsOn && ac->isEnabled(), out, seen);
            continue;
        }
        const QString label = plain(ac->text());
        if (label.isEmpty())
            continue;
        CommandFinder::Entry e;
        e.action = ac;
        e.label = label;
        e.path = path;
        e.shortcut = ac->shortcut().toString(QKeySequence::NativeText);
        e.enabled = parentsOn && ac->isEnabled();
        out->append(e);
    }
}

QList<CommandFinder::Entry> CommandFinder::collect(QMenuBar *bar)
{
    QList<Entry> out;
    QList<QMenu *> seen;
    for (QAction *top : bar->actions())
        if (QMenu *m = top->menu())
            walk(m, plain(m->title()), top->isEnabled(), &out, &seen);
    return out;
}

QList<int> CommandFinder::matching(const QList<Entry> &entries, const QString &text)
{
    const QStringList words = text.simplified().toCaseFolded().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    QList<int> rows;
    for (int i = 0; i < entries.size(); ++i) {
        const QString hay = (entries[i].label + QLatin1Char(' ') + entries[i].path).toCaseFolded();
        bool all = true;
        for (const QString &w : words)
            if (!hay.contains(w)) {
                all = false;
                break;
            }
        if (all)
            rows << i;
    }
    return rows;
}

CommandFinder::CommandFinder(QMenuBar *bar, QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("findCommandDialog"));
    setWindowTitle(tr("Find Command"));
    m_entries = collect(bar);
    auto *v = new QVBoxLayout(this);
    auto *hint = new QLabel(tr("Type part of a command's name or of its menu. Return or a double-click runs it."), this);
    hint->setWordWrap(true);
    v->addWidget(hint);
    m_search = new QLineEdit(this);
    m_search->setObjectName(QStringLiteral("commandSearch"));
    m_search->setClearButtonEnabled(true);
    m_search->setPlaceholderText(tr("Command"));
    v->addWidget(m_search);
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("commandList"));
    m_list->setUniformItemSizes(true);
    v->addWidget(m_list, 1);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Close, this);
    v->addWidget(bb);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_search, &QLineEdit::textChanged, this, [this] { refill(); });
    connect(m_search, &QLineEdit::returnPressed, this, [this] { runCurrent(); });
    connect(m_list, &QListWidget::itemActivated, this, [this](QListWidgetItem *it) {
        if (it)
            runRow(it->data(Qt::UserRole).toInt());
    });
    // Up and Down in the field move through the list
    m_search->installEventFilter(this);
    resize(640, 480);
    refill();
    m_search->setFocus();
}

void CommandFinder::refill()
{
    m_list->clear();
    for (int row : matching(m_entries, m_search->text())) {
        const Entry &e = m_entries[row];
        QString text = e.label + QStringLiteral("    ") + e.path;
        if (!e.shortcut.isEmpty())
            text += QStringLiteral("    [") + e.shortcut + QLatin1Char(']');
        auto *it = new QListWidgetItem(text, m_list);
        it->setData(Qt::UserRole, row);
        it->setToolTip(e.path + QLatin1String(" > ") + e.label);
        if (!e.enabled)
            it->setFlags(it->flags() & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
    }
    for (int i = 0; i < m_list->count(); ++i)
        if (m_list->item(i)->flags() & Qt::ItemIsEnabled) {
            m_list->setCurrentRow(i);
            break;
        }
}

void CommandFinder::step(int by)
{
    int r = m_list->currentRow();
    for (int n = 0; n < m_list->count(); ++n) {
        r += by;
        if (r < 0 || r >= m_list->count())
            return;
        if (m_list->item(r)->flags() & Qt::ItemIsEnabled) {
            m_list->setCurrentRow(r);
            return;
        }
    }
}

bool CommandFinder::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == m_search && ev->type() == QEvent::KeyPress) {
        const int k = static_cast<QKeyEvent *>(ev)->key();
        if (k == Qt::Key_Down || k == Qt::Key_Up) {
            step(k == Qt::Key_Down ? 1 : -1);
            return true;
        }
    }
    return QDialog::eventFilter(obj, ev);
}

void CommandFinder::setFilter(const QString &text) { m_search->setText(text); }

int CommandFinder::visibleRows() const { return m_list->count(); }

bool CommandFinder::runRow(int row)
{
    if (row < 0 || row >= m_entries.size())
        return false;
    const Entry e = m_entries[row];
    if (!e.action || !e.enabled || !e.action->isEnabled())
        return false;
    QPointer<QAction> ac = e.action;
    const QString path = e.path + QLatin1String(" > ") + e.label;
    hide();
    // after the window has gone: a command that asks something (a dialog) must not find this one in its way
    QTimer::singleShot(0, ac.data(), [ac] {
        if (ac && ac->isEnabled())
            ac->trigger();
    });
    emit commandRun(path);
    accept();
    return true;
}

bool CommandFinder::runCurrent()
{
    QListWidgetItem *it = m_list->currentItem();
    if (!it)
        return false;
    return runRow(it->data(Qt::UserRole).toInt());
}

} // namespace qb
