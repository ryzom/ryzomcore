#ifndef PROFILESMODEL_H
#define PROFILESMODEL_H

#include "configfile.h"

/**
 * Profiles model
 *
 * \author Cedric 'Kervala' OCHS
 * \date 2016
 */
class CProfilesModel : public QAbstractListModel
{
public:
	CProfilesModel(QObject *parent);
	CProfilesModel(const CProfiles &profiles, QObject *parent);
	virtual ~CProfilesModel();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

	bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());

	CProfiles& getProfiles() { return m_profiles; }

	bool save() const;

	int getIndexFromProfileID(const QString &profileId) const;
	QString getProfileIDFromIndex(int index) const;

private:
	CProfiles m_profiles;
};

#endif
