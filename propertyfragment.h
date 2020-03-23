#ifndef PROPERTYFRAGMENT_H
#define PROPERTYFRAGMENT_H

#include "fragment.h"

namespace Ui {
class PropertyFragment;
}

class PropertyFragment final : public Fragment {
    Q_OBJECT

public:
    explicit PropertyFragment(QWidget *parent = nullptr);
    ~PropertyFragment();

public slots:
    bool getDataAndFormula( const int &id );
    void getFormula();
    void getData();

    void sendFormulaRequest();
    void sendDataRequest();
    bool parseFormulaRequest( const QByteArray &data );
    bool parseDataRequest( const QByteArray &data );

    void readData( const QByteArray &uncompressed ) const;

    void readFormula( const QByteArray &data );

private:
    Ui::PropertyFragment *ui;
};

#endif // PROPERTYFRAGMENT_H
