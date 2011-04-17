#ifndef CONTACTLISTCONFERENCE_H
#define CONTACTLISTCONFERENCE_H

#include <QListView>

#include "contact.h"
#include "account.h"
#include "onlinestatus.h"
#include "conferencelistmodel.h"

class ContactListConference : public QListView
{
	Q_OBJECT
public:
	ContactListConference(Contact* conference, Account* acc, QWidget* parent = 0);
	~ContactListConference();

signals:
	void setMembersCount(quint32);

private slots:
	void addContact(QByteArray & contact);
	void onlineStatusChanged(OnlineStatus);

private:
	Account* m_account;
	Contact* m_conf;
	bool accountWasConnected;

	ConferenceListModel* m_model;
	QList<Contact*> m_contacts;

	quint32 membersCount;
};

#endif // CONTACTLISTCONFERENCE_H
