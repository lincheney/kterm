#include <QApplication>
#include <KParts/ReadOnlyPart>
#include <KService>

typedef KParts::ReadOnlyPart TermPart;

class TermApp: public QApplication
{
    Q_OBJECT
public:
    TermApp(int &argc, char **argv) : QApplication(argc, argv) {};
    KService::Ptr konsole_service();
    TermPart* make_term();

public Q_SLOTS:
    void slotTermActivityDetected();
    void slotTermSetWindowCaption(QString);

protected:
    void updateTermTitle(QObject*);

private:
    KService::Ptr m_service;
};
