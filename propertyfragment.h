#ifndef PROPERTYFRAGMENT_H
#define PROPERTYFRAGMENT_H

#include <QMainWindow>

namespace Ui {
class PropertyFragment;
}

class PropertyFragment : public QMainWindow
{
    Q_OBJECT

public:
    explicit PropertyFragment(QWidget *parent = nullptr);
    ~PropertyFragment();

private:
    Ui::PropertyFragment *ui;
};

#endif // PROPERTYFRAGMENT_H
