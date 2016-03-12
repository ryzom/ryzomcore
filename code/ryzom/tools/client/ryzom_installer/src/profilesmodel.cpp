#include "profilesmodel.h"

CProfilesModel::CProfilesModel(QObject *parent):QAbstractListModel(parent)
{
	m_profiles = CConfigFile::getInstance()->getProfiles();
}

CProfilesModel::CProfilesModel(const CProfiles &profiles, QObject *parent):QAbstractListModel(parent), m_profiles(profiles)
{
}

CProfilesModel::~CProfilesModel()
{
}

int CProfilesModel::rowCount(const QModelIndex &parent) const
{
	return m_profiles.size();
}

QVariant CProfilesModel::data(const QModelIndex &index, int role) const
{
	if (role != Qt::DisplayRole) return QVariant();

	const CProfile &profile = m_profiles.at(index.row());

	return QString("%1 (#%2)").arg(profile.name).arg(profile.id);
}

bool CProfilesModel::removeRows(int row, int count, const QModelIndex &parent)
{
	if (row < 0) return false;

	beginRemoveRows(parent, row, row + count - 1);

	m_profiles.removeAt(row);

	endRemoveRows();

	return true;
}

bool CProfilesModel::save() const
{
	CConfigFile::getInstance()->setProfiles(m_profiles);

	return true;
}
