/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of PySide2.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/


#ifndef SBK_QTNETWORK_PYTHON_H
#define SBK_QTNETWORK_PYTHON_H

#include <sbkpython.h>
#include <conversions.h>
#include <sbkenum.h>
#include <basewrapper.h>
#include <bindingmanager.h>
#include <memory>

#include <pysidesignal.h>
// Module Includes
#include <pyside2_qtcore_python.h>

// Binded library includes
#include <qnetworkproxy.h>
#include <qnetworkrequest.h>
#include <qauthenticator.h>
#include <qnetworkdiskcache.h>
#include <qnetworksession.h>
#include <qnetworkcookie.h>
#include <qabstractnetworkcache.h>
#include <qabstractsocket.h>
#include <qnetworkinterface.h>
#include <qhostaddress.h>
#include <qlocalsocket.h>
#include <qnetworkconfigmanager.h>
#include <qtcpsocket.h>
#include <qtcpserver.h>
#include <qnetworkreply.h>
#include <qlocalserver.h>
#include <qudpsocket.h>
#include <qnetworkcookiejar.h>
#include <qnetworkconfiguration.h>
#include <qnetworkaccessmanager.h>
#include <qhostinfo.h>
#include <qssl.h>
// Conversion Includes - Primitive Types
#include <QStringList>
#include <qabstractitemmodel.h>
#include <signalmanager.h>
#include <wtypes.h>
#include <typeresolver.h>
#include <QString>

// Conversion Includes - Container Types
#include <QMap>
#include <QStack>
#include <QLinkedList>
#include <QSet>
#include <QList>
#include <QVector>
#include <QMultiMap>
#include <QQueue>
#include <QPair>
#include <pysideconversions.h>

// Type indices
#define SBK_QSSL_IDX                                                 65
#define SBK_QSSL_KEYTYPE_IDX                                         69
#define SBK_QSSL_ENCODINGFORMAT_IDX                                  67
#define SBK_QSSL_KEYALGORITHM_IDX                                    68
#define SBK_QSSL_ALTERNATIVENAMEENTRYTYPE_IDX                        66
#define SBK_QSSL_SSLPROTOCOL_IDX                                     71
#define SBK_QSSL_SSLOPTION_IDX                                       70
#define SBK_QFLAGS_QSSL_SSLOPTION__IDX                               18
#define SBK_QNETWORKPROXY_IDX                                        47
#define SBK_QNETWORKPROXY_PROXYTYPE_IDX                              49
#define SBK_QNETWORKPROXY_CAPABILITY_IDX                             48
#define SBK_QFLAGS_QNETWORKPROXY_CAPABILITY__IDX                     16
#define SBK_QNETWORKPROXYQUERY_IDX                                   51
#define SBK_QNETWORKPROXYQUERY_QUERYTYPE_IDX                         52
#define SBK_QNETWORKPROXYFACTORY_IDX                                 50
#define SBK_QHOSTINFO_IDX                                            21
#define SBK_QHOSTINFO_HOSTINFOERROR_IDX                              22
#define SBK_QAUTHENTICATOR_IDX                                       9
#define SBK_QNETWORKINTERFACE_IDX                                    45
#define SBK_QNETWORKINTERFACE_INTERFACEFLAG_IDX                      46
#define SBK_QFLAGS_QNETWORKINTERFACE_INTERFACEFLAG__IDX              15
#define SBK_QNETWORKADDRESSENTRY_IDX                                 32
#define SBK_QHOSTADDRESS_IDX                                         19
#define SBK_QHOSTADDRESS_SPECIALADDRESS_IDX                          20
#define SBK_QIPV6ADDRESS_IDX                                         23
#define SBK_QNETWORKCONFIGURATION_IDX                                34
#define SBK_QNETWORKCONFIGURATION_TYPE_IDX                           38
#define SBK_QNETWORKCONFIGURATION_PURPOSE_IDX                        36
#define SBK_QNETWORKCONFIGURATION_STATEFLAG_IDX                      37
#define SBK_QFLAGS_QNETWORKCONFIGURATION_STATEFLAG__IDX              13
#define SBK_QNETWORKCONFIGURATION_BEARERTYPE_IDX                     35
#define SBK_QNETWORKCACHEMETADATA_IDX                                33
#define SBK_QABSTRACTNETWORKCACHE_IDX                                0
#define SBK_QNETWORKDISKCACHE_IDX                                    44
#define SBK_QTCPSERVER_IDX                                           72
#define SBK_QLOCALSERVER_IDX                                         24
#define SBK_QLOCALSERVER_SOCKETOPTION_IDX                            25
#define SBK_QFLAGS_QLOCALSERVER_SOCKETOPTION__IDX                    12
#define SBK_QNETWORKSESSION_IDX                                      61
#define SBK_QNETWORKSESSION_STATE_IDX                                63
#define SBK_QNETWORKSESSION_SESSIONERROR_IDX                         62
#define SBK_QNETWORKSESSION_USAGEPOLICY_IDX                          64
#define SBK_QFLAGS_QNETWORKSESSION_USAGEPOLICY__IDX                  17
#define SBK_QNETWORKREPLY_IDX                                        53
#define SBK_QNETWORKREPLY_NETWORKERROR_IDX                           54
#define SBK_QLOCALSOCKET_IDX                                         26
#define SBK_QLOCALSOCKET_LOCALSOCKETERROR_IDX                        27
#define SBK_QLOCALSOCKET_LOCALSOCKETSTATE_IDX                        28
#define SBK_QABSTRACTSOCKET_IDX                                      1
#define SBK_QABSTRACTSOCKET_SOCKETTYPE_IDX                           8
#define SBK_QABSTRACTSOCKET_NETWORKLAYERPROTOCOL_IDX                 3
#define SBK_QABSTRACTSOCKET_SOCKETERROR_IDX                          5
#define SBK_QABSTRACTSOCKET_SOCKETSTATE_IDX                          7
#define SBK_QABSTRACTSOCKET_SOCKETOPTION_IDX                         6
#define SBK_QABSTRACTSOCKET_BINDFLAG_IDX                             2
#define SBK_QFLAGS_QABSTRACTSOCKET_BINDFLAG__IDX                     10
#define SBK_QABSTRACTSOCKET_PAUSEMODE_IDX                            4
#define SBK_QFLAGS_QABSTRACTSOCKET_PAUSEMODE__IDX                    11
#define SBK_QUDPSOCKET_IDX                                           74
#define SBK_QTCPSOCKET_IDX                                           73
#define SBK_QNETWORKCONFIGURATIONMANAGER_IDX                         39
#define SBK_QNETWORKCONFIGURATIONMANAGER_CAPABILITY_IDX              40
#define SBK_QFLAGS_QNETWORKCONFIGURATIONMANAGER_CAPABILITY__IDX      14
#define SBK_QNETWORKCOOKIEJAR_IDX                                    43
#define SBK_QNETWORKACCESSMANAGER_IDX                                29
#define SBK_QNETWORKACCESSMANAGER_OPERATION_IDX                      31
#define SBK_QNETWORKACCESSMANAGER_NETWORKACCESSIBILITY_IDX           30
#define SBK_QNETWORKREQUEST_IDX                                      55
#define SBK_QNETWORKREQUEST_KNOWNHEADERS_IDX                         58
#define SBK_QNETWORKREQUEST_ATTRIBUTE_IDX                            56
#define SBK_QNETWORKREQUEST_CACHELOADCONTROL_IDX                     57
#define SBK_QNETWORKREQUEST_LOADCONTROL_IDX                          59
#define SBK_QNETWORKREQUEST_PRIORITY_IDX                             60
#define SBK_QNETWORKCOOKIE_IDX                                       41
#define SBK_QNETWORKCOOKIE_RAWFORM_IDX                               42
#define SBK_QtNetwork_IDX_COUNT                                      75

// This variable stores all Python types exported by this module.
extern PyTypeObject** SbkPySide2_QtNetworkTypes;

// This variable stores all type converters exported by this module.
extern SbkConverter** SbkPySide2_QtNetworkTypeConverters;

// Converter indices
#define SBK_QTNETWORK_QLIST_QBYTEARRAY_IDX                           0 // QList<QByteArray >
#define SBK_QTNETWORK_QLIST_QNETWORKPROXY_IDX                        1 // QList<QNetworkProxy >
#define SBK_QTNETWORK_QLIST_QHOSTADDRESS_IDX                         2 // QList<QHostAddress >
#define SBK_QTNETWORK_QHASH_QSTRING_QVARIANT_IDX                     3 // QHash<QString, QVariant >
#define SBK_QTNETWORK_QLIST_QNETWORKADDRESSENTRY_IDX                 4 // QList<QNetworkAddressEntry >
#define SBK_QTNETWORK_QLIST_QNETWORKINTERFACE_IDX                    5 // QList<QNetworkInterface >
#define SBK_QTNETWORK_QPAIR_QHOSTADDRESS_INT_IDX                     6 // const QPair<QHostAddress, int > &
#define SBK_QTNETWORK_QLIST_QNETWORKCONFIGURATION_IDX                7 // QList<QNetworkConfiguration >
#define SBK_QTNETWORK_QHASH_QNETWORKREQUEST_ATTRIBUTE_QVARIANT_IDX   8 // QHash<QNetworkRequest::Attribute, QVariant >
#define SBK_QTNETWORK_QPAIR_QBYTEARRAY_QBYTEARRAY_IDX                9 // QPair<QByteArray, QByteArray >
#define SBK_QTNETWORK_QLIST_QPAIR_QBYTEARRAY_QBYTEARRAY_IDX          10 // QList<QPair<QByteArray, QByteArray > >
#define SBK_QTNETWORK_QLIST_QOBJECTPTR_IDX                           11 // const QList<QObject * > &
#define SBK_QTNETWORK_QLIST_QNETWORKCOOKIE_IDX                       12 // QList<QNetworkCookie >
#define SBK_QTNETWORK_QLIST_QVARIANT_IDX                             13 // QList<QVariant >
#define SBK_QTNETWORK_QLIST_QSTRING_IDX                              14 // QList<QString >
#define SBK_QTNETWORK_QMAP_QSTRING_QVARIANT_IDX                      15 // QMap<QString, QVariant >
#define SBK_QtNetwork_CONVERTERS_IDX_COUNT                           16

// Macros for type check

namespace Shiboken
{

// PyType functions, to get the PyObjectType for a type T
template<> inline PyTypeObject* SbkType< ::QSsl::KeyType >() { return SbkPySide2_QtNetworkTypes[SBK_QSSL_KEYTYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QSsl::EncodingFormat >() { return SbkPySide2_QtNetworkTypes[SBK_QSSL_ENCODINGFORMAT_IDX]; }
template<> inline PyTypeObject* SbkType< ::QSsl::KeyAlgorithm >() { return SbkPySide2_QtNetworkTypes[SBK_QSSL_KEYALGORITHM_IDX]; }
template<> inline PyTypeObject* SbkType< ::QSsl::AlternativeNameEntryType >() { return SbkPySide2_QtNetworkTypes[SBK_QSSL_ALTERNATIVENAMEENTRYTYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QSsl::SslProtocol >() { return SbkPySide2_QtNetworkTypes[SBK_QSSL_SSLPROTOCOL_IDX]; }
template<> inline PyTypeObject* SbkType< ::QSsl::SslOption >() { return SbkPySide2_QtNetworkTypes[SBK_QSSL_SSLOPTION_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QSsl::SslOption> >() { return SbkPySide2_QtNetworkTypes[SBK_QFLAGS_QSSL_SSLOPTION__IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkProxy::ProxyType >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKPROXY_PROXYTYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkProxy::Capability >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKPROXY_CAPABILITY_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QNetworkProxy::Capability> >() { return SbkPySide2_QtNetworkTypes[SBK_QFLAGS_QNETWORKPROXY_CAPABILITY__IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkProxy >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QNETWORKPROXY_IDX]); }
template<> inline PyTypeObject* SbkType< ::QNetworkProxyQuery::QueryType >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKPROXYQUERY_QUERYTYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkProxyQuery >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QNETWORKPROXYQUERY_IDX]); }
template<> inline PyTypeObject* SbkType< ::QNetworkProxyFactory >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QNETWORKPROXYFACTORY_IDX]); }
template<> inline PyTypeObject* SbkType< ::QHostInfo::HostInfoError >() { return SbkPySide2_QtNetworkTypes[SBK_QHOSTINFO_HOSTINFOERROR_IDX]; }
template<> inline PyTypeObject* SbkType< ::QHostInfo >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QHOSTINFO_IDX]); }
template<> inline PyTypeObject* SbkType< ::QAuthenticator >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QAUTHENTICATOR_IDX]); }
template<> inline PyTypeObject* SbkType< ::QNetworkInterface::InterfaceFlag >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKINTERFACE_INTERFACEFLAG_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QNetworkInterface::InterfaceFlag> >() { return SbkPySide2_QtNetworkTypes[SBK_QFLAGS_QNETWORKINTERFACE_INTERFACEFLAG__IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkInterface >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QNETWORKINTERFACE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QNetworkAddressEntry >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QNETWORKADDRESSENTRY_IDX]); }
template<> inline PyTypeObject* SbkType< ::QHostAddress::SpecialAddress >() { return SbkPySide2_QtNetworkTypes[SBK_QHOSTADDRESS_SPECIALADDRESS_IDX]; }
template<> inline PyTypeObject* SbkType< ::QHostAddress >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QHOSTADDRESS_IDX]); }
template<> inline PyTypeObject* SbkType< ::QIPv6Address >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QIPV6ADDRESS_IDX]); }
template<> inline PyTypeObject* SbkType< ::QNetworkConfiguration::Type >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKCONFIGURATION_TYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkConfiguration::Purpose >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKCONFIGURATION_PURPOSE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkConfiguration::StateFlag >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKCONFIGURATION_STATEFLAG_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QNetworkConfiguration::StateFlag> >() { return SbkPySide2_QtNetworkTypes[SBK_QFLAGS_QNETWORKCONFIGURATION_STATEFLAG__IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkConfiguration::BearerType >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKCONFIGURATION_BEARERTYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkConfiguration >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QNETWORKCONFIGURATION_IDX]); }
template<> inline PyTypeObject* SbkType< ::QNetworkCacheMetaData >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QNETWORKCACHEMETADATA_IDX]); }
template<> inline PyTypeObject* SbkType< ::QAbstractNetworkCache >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QABSTRACTNETWORKCACHE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QNetworkDiskCache >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QNETWORKDISKCACHE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTcpServer >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QTCPSERVER_IDX]); }
template<> inline PyTypeObject* SbkType< ::QLocalServer::SocketOption >() { return SbkPySide2_QtNetworkTypes[SBK_QLOCALSERVER_SOCKETOPTION_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QLocalServer::SocketOption> >() { return SbkPySide2_QtNetworkTypes[SBK_QFLAGS_QLOCALSERVER_SOCKETOPTION__IDX]; }
template<> inline PyTypeObject* SbkType< ::QLocalServer >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QLOCALSERVER_IDX]); }
template<> inline PyTypeObject* SbkType< ::QNetworkSession::State >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKSESSION_STATE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkSession::SessionError >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKSESSION_SESSIONERROR_IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkSession::UsagePolicy >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKSESSION_USAGEPOLICY_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QNetworkSession::UsagePolicy> >() { return SbkPySide2_QtNetworkTypes[SBK_QFLAGS_QNETWORKSESSION_USAGEPOLICY__IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkSession >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QNETWORKSESSION_IDX]); }
template<> inline PyTypeObject* SbkType< ::QNetworkReply::NetworkError >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKREPLY_NETWORKERROR_IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkReply >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QNETWORKREPLY_IDX]); }
template<> inline PyTypeObject* SbkType< ::QLocalSocket::LocalSocketError >() { return SbkPySide2_QtNetworkTypes[SBK_QLOCALSOCKET_LOCALSOCKETERROR_IDX]; }
template<> inline PyTypeObject* SbkType< ::QLocalSocket::LocalSocketState >() { return SbkPySide2_QtNetworkTypes[SBK_QLOCALSOCKET_LOCALSOCKETSTATE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QLocalSocket >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QLOCALSOCKET_IDX]); }
template<> inline PyTypeObject* SbkType< ::QAbstractSocket::SocketType >() { return SbkPySide2_QtNetworkTypes[SBK_QABSTRACTSOCKET_SOCKETTYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAbstractSocket::NetworkLayerProtocol >() { return SbkPySide2_QtNetworkTypes[SBK_QABSTRACTSOCKET_NETWORKLAYERPROTOCOL_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAbstractSocket::SocketError >() { return SbkPySide2_QtNetworkTypes[SBK_QABSTRACTSOCKET_SOCKETERROR_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAbstractSocket::SocketState >() { return SbkPySide2_QtNetworkTypes[SBK_QABSTRACTSOCKET_SOCKETSTATE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAbstractSocket::SocketOption >() { return SbkPySide2_QtNetworkTypes[SBK_QABSTRACTSOCKET_SOCKETOPTION_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAbstractSocket::BindFlag >() { return SbkPySide2_QtNetworkTypes[SBK_QABSTRACTSOCKET_BINDFLAG_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QAbstractSocket::BindFlag> >() { return SbkPySide2_QtNetworkTypes[SBK_QFLAGS_QABSTRACTSOCKET_BINDFLAG__IDX]; }
template<> inline PyTypeObject* SbkType< ::QAbstractSocket::PauseMode >() { return SbkPySide2_QtNetworkTypes[SBK_QABSTRACTSOCKET_PAUSEMODE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QAbstractSocket::PauseMode> >() { return SbkPySide2_QtNetworkTypes[SBK_QFLAGS_QABSTRACTSOCKET_PAUSEMODE__IDX]; }
template<> inline PyTypeObject* SbkType< ::QAbstractSocket >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QABSTRACTSOCKET_IDX]); }
template<> inline PyTypeObject* SbkType< ::QUdpSocket >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QUDPSOCKET_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTcpSocket >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QTCPSOCKET_IDX]); }
template<> inline PyTypeObject* SbkType< ::QNetworkConfigurationManager::Capability >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKCONFIGURATIONMANAGER_CAPABILITY_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QNetworkConfigurationManager::Capability> >() { return SbkPySide2_QtNetworkTypes[SBK_QFLAGS_QNETWORKCONFIGURATIONMANAGER_CAPABILITY__IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkConfigurationManager >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QNETWORKCONFIGURATIONMANAGER_IDX]); }
template<> inline PyTypeObject* SbkType< ::QNetworkCookieJar >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QNETWORKCOOKIEJAR_IDX]); }
template<> inline PyTypeObject* SbkType< ::QNetworkAccessManager::Operation >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKACCESSMANAGER_OPERATION_IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkAccessManager::NetworkAccessibility >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKACCESSMANAGER_NETWORKACCESSIBILITY_IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkAccessManager >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QNETWORKACCESSMANAGER_IDX]); }
template<> inline PyTypeObject* SbkType< ::QNetworkRequest::KnownHeaders >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKREQUEST_KNOWNHEADERS_IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkRequest::Attribute >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKREQUEST_ATTRIBUTE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkRequest::CacheLoadControl >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKREQUEST_CACHELOADCONTROL_IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkRequest::LoadControl >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKREQUEST_LOADCONTROL_IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkRequest::Priority >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKREQUEST_PRIORITY_IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkRequest >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QNETWORKREQUEST_IDX]); }
template<> inline PyTypeObject* SbkType< ::QNetworkCookie::RawForm >() { return SbkPySide2_QtNetworkTypes[SBK_QNETWORKCOOKIE_RAWFORM_IDX]; }
template<> inline PyTypeObject* SbkType< ::QNetworkCookie >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtNetworkTypes[SBK_QNETWORKCOOKIE_IDX]); }

} // namespace Shiboken

#endif // SBK_QTNETWORK_PYTHON_H

