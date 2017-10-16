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


#ifndef SBK_QTMULTIMEDIA_PYTHON_H
#define SBK_QTMULTIMEDIA_PYTHON_H

#include <sbkpython.h>
#include <conversions.h>
#include <sbkenum.h>
#include <basewrapper.h>
#include <bindingmanager.h>
#include <memory>

#include <pysidesignal.h>
// Module Includes
#include <pyside2_qtcore_python.h>
#include <pyside2_qtgui_python.h>
#include <pyside2_qtnetwork_python.h>

// Binded library includes
#include <qabstractvideobuffer.h>
#include <qaudiodeviceinfo.h>
#include <qaudiosystem.h>
#include <qaudioinput.h>
#include <qaudiooutput.h>
#include <qvideosurfaceformat.h>
#include <qabstractvideosurface.h>
#include <qaudioformat.h>
#include <qaudio.h>
#include <qvideoframe.h>
// Conversion Includes - Primitive Types
#include <typeresolver.h>
#include <signalmanager.h>
#include <wtypes.h>
#include <qabstractitemmodel.h>
#include <QString>
#include <QStringList>

// Conversion Includes - Container Types
#include <QVector>
#include <QList>
#include <QLinkedList>
#include <QMap>
#include <QSet>
#include <pysideconversions.h>
#include <QMultiMap>
#include <QQueue>
#include <QStack>
#include <QPair>

// Type indices
#define SBK_QAUDIO_IDX                                               8
#define SBK_QAUDIO_ERROR_IDX                                         9
#define SBK_QAUDIO_STATE_IDX                                         11
#define SBK_QAUDIO_MODE_IDX                                          10
#define SBK_QAUDIODEVICEINFO_IDX                                     12
#define SBK_QAUDIOFORMAT_IDX                                         13
#define SBK_QAUDIOFORMAT_SAMPLETYPE_IDX                              15
#define SBK_QAUDIOFORMAT_ENDIAN_IDX                                  14
#define SBK_QVIDEOFRAME_IDX                                          18
#define SBK_QVIDEOFRAME_FIELDTYPE_IDX                                19
#define SBK_QVIDEOFRAME_PIXELFORMAT_IDX                              20
#define SBK_QABSTRACTVIDEOBUFFER_IDX                                 3
#define SBK_QABSTRACTVIDEOBUFFER_HANDLETYPE_IDX                      4
#define SBK_QABSTRACTVIDEOBUFFER_MAPMODE_IDX                         5
#define SBK_QVIDEOSURFACEFORMAT_IDX                                  21
#define SBK_QVIDEOSURFACEFORMAT_DIRECTION_IDX                        22
#define SBK_QVIDEOSURFACEFORMAT_YCBCRCOLORSPACE_IDX                  23
#define SBK_QABSTRACTAUDIODEVICEINFO_IDX                             0
#define SBK_QABSTRACTAUDIOOUTPUT_IDX                                 2
#define SBK_QAUDIOINPUT_IDX                                          16
#define SBK_QAUDIOOUTPUT_IDX                                         17
#define SBK_QABSTRACTVIDEOSURFACE_IDX                                6
#define SBK_QABSTRACTVIDEOSURFACE_ERROR_IDX                          7
#define SBK_QABSTRACTAUDIOINPUT_IDX                                  1
#define SBK_QtMultimedia_IDX_COUNT                                   24

// This variable stores all Python types exported by this module.
extern PyTypeObject** SbkPySide2_QtMultimediaTypes;

// This variable stores all type converters exported by this module.
extern SbkConverter** SbkPySide2_QtMultimediaTypeConverters;

// Converter indices
#define SBK_QTMULTIMEDIA_QLIST_QAUDIODEVICEINFO_IDX                  0 // QList<QAudioDeviceInfo >
#define SBK_QTMULTIMEDIA_QLIST_QAUDIOFORMAT_ENDIAN_IDX               1 // QList<QAudioFormat::Endian >
#define SBK_QTMULTIMEDIA_QLIST_INT_IDX                               2 // QList<int >
#define SBK_QTMULTIMEDIA_QLIST_QAUDIOFORMAT_SAMPLETYPE_IDX           3 // QList<QAudioFormat::SampleType >
#define SBK_QTMULTIMEDIA_QLIST_QBYTEARRAY_IDX                        4 // QList<QByteArray >
#define SBK_QTMULTIMEDIA_QLIST_QOBJECTPTR_IDX                        5 // const QList<QObject * > &
#define SBK_QTMULTIMEDIA_QLIST_QVIDEOFRAME_PIXELFORMAT_IDX           6 // QList<QVideoFrame::PixelFormat >
#define SBK_QTMULTIMEDIA_QLIST_QVARIANT_IDX                          7 // QList<QVariant >
#define SBK_QTMULTIMEDIA_QLIST_QSTRING_IDX                           8 // QList<QString >
#define SBK_QTMULTIMEDIA_QMAP_QSTRING_QVARIANT_IDX                   9 // QMap<QString, QVariant >
#define SBK_QtMultimedia_CONVERTERS_IDX_COUNT                        10

// Macros for type check

namespace Shiboken
{

// PyType functions, to get the PyObjectType for a type T
template<> inline PyTypeObject* SbkType< ::QAudio::Error >() { return SbkPySide2_QtMultimediaTypes[SBK_QAUDIO_ERROR_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAudio::State >() { return SbkPySide2_QtMultimediaTypes[SBK_QAUDIO_STATE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAudio::Mode >() { return SbkPySide2_QtMultimediaTypes[SBK_QAUDIO_MODE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAudioDeviceInfo >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtMultimediaTypes[SBK_QAUDIODEVICEINFO_IDX]); }
template<> inline PyTypeObject* SbkType< ::QAudioFormat::SampleType >() { return SbkPySide2_QtMultimediaTypes[SBK_QAUDIOFORMAT_SAMPLETYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAudioFormat::Endian >() { return SbkPySide2_QtMultimediaTypes[SBK_QAUDIOFORMAT_ENDIAN_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAudioFormat >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtMultimediaTypes[SBK_QAUDIOFORMAT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QVideoFrame::FieldType >() { return SbkPySide2_QtMultimediaTypes[SBK_QVIDEOFRAME_FIELDTYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QVideoFrame::PixelFormat >() { return SbkPySide2_QtMultimediaTypes[SBK_QVIDEOFRAME_PIXELFORMAT_IDX]; }
template<> inline PyTypeObject* SbkType< ::QVideoFrame >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtMultimediaTypes[SBK_QVIDEOFRAME_IDX]); }
template<> inline PyTypeObject* SbkType< ::QAbstractVideoBuffer::HandleType >() { return SbkPySide2_QtMultimediaTypes[SBK_QABSTRACTVIDEOBUFFER_HANDLETYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAbstractVideoBuffer::MapMode >() { return SbkPySide2_QtMultimediaTypes[SBK_QABSTRACTVIDEOBUFFER_MAPMODE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAbstractVideoBuffer >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtMultimediaTypes[SBK_QABSTRACTVIDEOBUFFER_IDX]); }
template<> inline PyTypeObject* SbkType< ::QVideoSurfaceFormat::Direction >() { return SbkPySide2_QtMultimediaTypes[SBK_QVIDEOSURFACEFORMAT_DIRECTION_IDX]; }
template<> inline PyTypeObject* SbkType< ::QVideoSurfaceFormat::YCbCrColorSpace >() { return SbkPySide2_QtMultimediaTypes[SBK_QVIDEOSURFACEFORMAT_YCBCRCOLORSPACE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QVideoSurfaceFormat >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtMultimediaTypes[SBK_QVIDEOSURFACEFORMAT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QAbstractAudioDeviceInfo >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtMultimediaTypes[SBK_QABSTRACTAUDIODEVICEINFO_IDX]); }
template<> inline PyTypeObject* SbkType< ::QAbstractAudioOutput >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtMultimediaTypes[SBK_QABSTRACTAUDIOOUTPUT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QAudioInput >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtMultimediaTypes[SBK_QAUDIOINPUT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QAudioOutput >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtMultimediaTypes[SBK_QAUDIOOUTPUT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QAbstractVideoSurface::Error >() { return SbkPySide2_QtMultimediaTypes[SBK_QABSTRACTVIDEOSURFACE_ERROR_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAbstractVideoSurface >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtMultimediaTypes[SBK_QABSTRACTVIDEOSURFACE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QAbstractAudioInput >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtMultimediaTypes[SBK_QABSTRACTAUDIOINPUT_IDX]); }

} // namespace Shiboken

#endif // SBK_QTMULTIMEDIA_PYTHON_H

