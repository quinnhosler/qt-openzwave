//-----------------------------------------------------------------------------
//
//	qtozwnodemodel_p.cpp
//
//	Node Model - Internal Class to Manage the Model
//
//	Copyright (c) 2019 Justin Hammond <Justin@dynam.ac>
//
//	SOFTWARE NOTICE AND LICENSE
//
//	This file is part of QT-OpenZWave.
//
//	OpenZWave is free software: you can redistribute it and/or modify
//	it under the terms of the GNU Lesser General Public License as published
//	by the Free Software Foundation, either version 3 of the License,
//	or (at your option) any later version.
//
//	OpenZWave is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU Lesser General Public License for more details.
//
//	You should have received a copy of the GNU Lesser General Public License
//	along with OpenZWave.  If not, see <http://www.gnu.org/licenses/>.
//
//-----------------------------------------------------------------------------

#include <QBitArray>
#include "qtozw_logging.h"
#include "qtozwnodemodel_p.h"


QTOZW_Nodes_internal::QTOZW_Nodes_internal(QObject *parent)
    : QTOZW_Nodes(parent)
{

}

void QTOZW_Nodes_internal::addNode(quint8 _nodeID)
{
    if (this->getNodeRow(_nodeID) >= 0) {
        qCWarning(nodeModel) << "Node " << _nodeID << " Already Exists";
        return;
    }
    QMap<QTOZW_Nodes::NodeColumns, QVariant> newNode;
    newNode[QTOZW_Nodes::NodeID] = _nodeID;
    QBitArray flags(static_cast<int>(QTOZW_Nodes::flagCount));
    newNode[QTOZW_Nodes::NodeFlags] = flags;

    this->beginInsertRows(QModelIndex(), this->rowCount(QModelIndex()), this->rowCount(QModelIndex()));
    this->m_nodeData.push_back(newNode);
    this->endInsertRows();
    qCInfo(nodeModel) << "Adding Node " << _nodeID << "At Row " << this->getNodeRow(_nodeID);
}

void QTOZW_Nodes_internal::setNodeData(quint8 _nodeID, QTOZW_Nodes::NodeColumns column, QVariant data, bool transaction)
{
    int row = this->getNodeRow(_nodeID);
    if (row == -1) {
        qCWarning(nodeModel) << "setNodeData: Node " << _nodeID << " does not exist";
        return;
    }
    if (this->m_nodeData.at(row)[column] != data) {
        this->m_nodeData[row][column] = data;
        QVector<int> roles;
        roles << Qt::DisplayRole;
        if (!transaction) this->dataChanged(this->createIndex(row, column), this->createIndex(row, column), roles);
        qCDebug(nodeModel) << "setNodeData " << _nodeID << ": " << column << ":" << data << " Transaction:" << transaction;

    }
}

void QTOZW_Nodes_internal::setNodeFlags(quint8 _nodeID, QTOZW_Nodes::nodeFlags _flags, bool _value, bool transaction)
{
    int row = this->getNodeRow(_nodeID);
    if (row == -1) {
        qCWarning(nodeModel) << "setNodeData: Node " << _nodeID << " does not exist";
        return;
    }
    QBitArray flag = this->m_nodeData.at(row)[QTOZW_Nodes::NodeFlags].toBitArray();
    if (flag.at(_flags) != _value) {
        flag.setBit(_flags, _value);
        this->m_nodeData[row][QTOZW_Nodes::NodeFlags] = flag;
        QVector<int> roles;
        roles << Qt::DisplayRole;
        if (!transaction) this->dataChanged(this->createIndex(row, QTOZW_Nodes::NodeFlags), this->createIndex(row, QTOZW_Nodes::NodeFlags), roles);
    }
}
void QTOZW_Nodes_internal::delNode(quint8 _nodeID) {
    QVector< QMap<NodeColumns, QVariant> >::iterator it;
    int i = 0;
    for (it = this->m_nodeData.begin(); it != this->m_nodeData.end();) {
        if ((*it)[QTOZW_Nodes::NodeColumns::NodeID] == _nodeID) {
            qCDebug(nodeModel) << "Removing Node " << (*it)[QTOZW_Nodes::NodeColumns::NodeID] << i;
            this->beginRemoveRows(QModelIndex(), i, i);
            it = this->m_nodeData.erase(it);
            this->endRemoveRows();
            continue;
        } else {
            it++;
        }
        i++;
    }
}

void QTOZW_Nodes_internal::resetModel() {
    this->beginRemoveRows(QModelIndex(), 0, this->m_nodeData.count());
    this->m_nodeData.clear();
    this->endRemoveRows();
}

void QTOZW_Nodes_internal::finishTransaction(quint8 _nodeID) {
    int row = this->getNodeRow(_nodeID);
    if (row == -1) {
        qCWarning(nodeModel) << "finishTransaction: Node " << _nodeID << " does not exist";
        return;
    }
    this->dataChanged(this->createIndex(row, 0), this->createIndex(row, QTOZW_Nodes::NodeColumns::NodeCount-1));

}

