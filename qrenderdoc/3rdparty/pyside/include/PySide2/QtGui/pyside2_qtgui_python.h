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


#ifndef SBK_QTGUI_PYTHON_H
#define SBK_QTGUI_PYTHON_H

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
#include <qfont.h>
#include <qclipboard.h>
#include <qregion.h>
#include <qpagedpaintdevice.h>
#include <qmatrix.h>
#include <qsurface.h>
#include <QTextLayout>
#include <qpainterpath.h>
#include <qopenglbuffer.h>
#include <qgenericmatrix.h>
#include <qtextlayout.h>
#include <qiconengine.h>
#include <qimagewriter.h>
#include <qpaintengine.h>
#include <QAbstractTextDocumentLayout>
#include <qtextdocument.h>
#include <qfontinfo.h>
#include <QInputMethodEvent>
#include <QPainterPath>
#include <qdrag.h>
#include <qtextobject.h>
#include <qtouchdevice.h>
#include <qbitmap.h>
#include <qguiapplication.h>
#include <qpainter.h>
#include <qtexttable.h>
#include <qevent.h>
#include <qwindow.h>
#include <qimageiohandler.h>
#include <qfontmetrics.h>
#include <qbrush.h>
#include <qsurfaceformat.h>
#include <QTextBlock>
#include <qopenglcontext.h>
#include <qkeysequence.h>
#include <qtextcursor.h>
#include <qdesktopservices.h>
#include <qvector3d.h>
#include <qpytextobject.h>
#include <qpaintdevice.h>
#include <qtextlist.h>
#include <qsyntaxhighlighter.h>
#include <qvalidator.h>
#include <qicon.h>
#include <qpicture.h>
#include <qmatrix4x4.h>
#include <qtextformat.h>
#include <qaccessible.h>
#include <qscreen.h>
#include <qopenglshaderprogram.h>
#include <qsessionmanager.h>
#include <qcolor.h>
#include <qvector2d.h>
#include <qabstracttextdocumentlayout.h>
#include <qpen.h>
#include <qtextdocumentwriter.h>
#include <QTextFrame>
#include <qstandarditemmodel.h>
#include <qstringlistmodel.h>
#include <qopenglframebufferobject.h>
#include <qimage.h>
#include <qpolygon.h>
#include <qquaternion.h>
#include <qtextoption.h>
#include <qimagereader.h>
#include <qpalette.h>
#include <qtextdocumentfragment.h>
#include <qpixmap.h>
#include <QPainter>
#include <qfontdatabase.h>
#include <qmovie.h>
#include <qpixmapcache.h>
#include <qtransform.h>
#include <qcursor.h>
#include <qvector4d.h>
// Conversion Includes - Primitive Types
#include <typeresolver.h>
#include <signalmanager.h>
#include <wtypes.h>
#include <QString>
#include <qabstractitemmodel.h>
#include <QStringList>

// Conversion Includes - Container Types
#include <QPair>
#include <QLinkedList>
#include <QQueue>
#include <QVector>
#include <QMultiMap>
#include <pysideconversions.h>
#include <QMap>
#include <QStack>
#include <QSet>
#include <QList>

// Type indices
#define SBK_QMATRIX4X3_IDX                                           110
#define SBK_QMATRIX4X2_IDX                                           109
#define SBK_QMATRIX3X4_IDX                                           108
#define SBK_QMATRIX3X3_IDX                                           107
#define SBK_QMATRIX3X2_IDX                                           106
#define SBK_QMATRIX2X4_IDX                                           105
#define SBK_QMATRIX2X3_IDX                                           104
#define SBK_QMATRIX2X2_IDX                                           103
#define SBK_QDESKTOPSERVICES_IDX                                     26
#define SBK_QTEXTTABLECELL_IDX                                       243
#define SBK_QTEXTFRAGMENT_IDX                                        214
#define SBK_QTEXTBLOCK_IDX                                           189
#define SBK_QTEXTBLOCK_ITERATOR_IDX                                  190
#define SBK_QTEXTBLOCKUSERDATA_IDX                                   193
#define SBK_QTEXTOBJECTINTERFACE_IDX                                 236
#define SBK_QTEXTLINE_IDX                                            229
#define SBK_QTEXTLINE_EDGE_IDX                                       231
#define SBK_QTEXTLINE_CURSORPOSITION_IDX                             230
#define SBK_QTEXTINLINEOBJECT_IDX                                    221
#define SBK_QTEXTCURSOR_IDX                                          198
#define SBK_QTEXTCURSOR_MOVEMODE_IDX                                 199
#define SBK_QTEXTCURSOR_MOVEOPERATION_IDX                            200
#define SBK_QTEXTCURSOR_SELECTIONTYPE_IDX                            201
#define SBK_QFONTDATABASE_IDX                                        59
#define SBK_QFONTDATABASE_WRITINGSYSTEM_IDX                          61
#define SBK_QFONTDATABASE_SYSTEMFONT_IDX                             60
#define SBK_QTEXTFORMAT_IDX                                          209
#define SBK_QTEXTFORMAT_FORMATTYPE_IDX                               210
#define SBK_QTEXTFORMAT_PROPERTY_IDX                                 213
#define SBK_QTEXTFORMAT_OBJECTTYPES_IDX                              211
#define SBK_QTEXTFORMAT_PAGEBREAKFLAG_IDX                            212
#define SBK_QFLAGS_QTEXTFORMAT_PAGEBREAKFLAG__IDX                    45
#define SBK_QTEXTBLOCKFORMAT_IDX                                     191
#define SBK_QTEXTBLOCKFORMAT_LINEHEIGHTTYPES_IDX                     267
#define SBK_QTEXTLISTFORMAT_IDX                                      233
#define SBK_QTEXTLISTFORMAT_STYLE_IDX                                234
#define SBK_QTEXTFRAMEFORMAT_IDX                                     217
#define SBK_QTEXTFRAMEFORMAT_POSITION_IDX                            219
#define SBK_QTEXTFRAMEFORMAT_BORDERSTYLE_IDX                         218
#define SBK_QTEXTTABLEFORMAT_IDX                                     245
#define SBK_QTEXTLENGTH_IDX                                          227
#define SBK_QTEXTLENGTH_TYPE_IDX                                     228
#define SBK_QPAINTENGINESTATE_IDX                                    138
#define SBK_QPAINTENGINE_IDX                                         133
#define SBK_QPAINTENGINE_PAINTENGINEFEATURE_IDX                      135
#define SBK_QFLAGS_QPAINTENGINE_PAINTENGINEFEATURE__IDX              40
#define SBK_QPAINTENGINE_DIRTYFLAG_IDX                               134
#define SBK_QFLAGS_QPAINTENGINE_DIRTYFLAG__IDX                       39
#define SBK_QPAINTENGINE_POLYGONDRAWMODE_IDX                         136
#define SBK_QPAINTENGINE_TYPE_IDX                                    137
#define SBK_QTEXTITEM_IDX                                            222
#define SBK_QTEXTITEM_RENDERFLAG_IDX                                 223
#define SBK_QFLAGS_QTEXTITEM_RENDERFLAG__IDX                         46
#define SBK_QFONTMETRICSF_IDX                                        64
#define SBK_QFONTMETRICS_IDX                                         63
#define SBK_QFONTINFO_IDX                                            62
#define SBK_QPEN_IDX                                                 152
#define SBK_QTEXTOPTION_IDX                                          237
#define SBK_QTEXTOPTION_TABTYPE_IDX                                  240
#define SBK_QTEXTOPTION_WRAPMODE_IDX                                 241
#define SBK_QTEXTOPTION_FLAG_IDX                                     238
#define SBK_QFLAGS_QTEXTOPTION_FLAG__IDX                             47
#define SBK_QTEXTOPTION_TAB_IDX                                      239
#define SBK_QOPENGLFRAMEBUFFEROBJECT_IDX                             124
#define SBK_QOPENGLFRAMEBUFFEROBJECT_ATTACHMENT_IDX                  125
#define SBK_QOPENGLBUFFER_IDX                                        117
#define SBK_QOPENGLBUFFER_TYPE_IDX                                   120
#define SBK_QOPENGLBUFFER_USAGEPATTERN_IDX                           121
#define SBK_QOPENGLBUFFER_ACCESS_IDX                                 118
#define SBK_QOPENGLBUFFER_RANGEACCESSFLAG_IDX                        119
#define SBK_QFLAGS_QOPENGLBUFFER_RANGEACCESSFLAG__IDX                37
#define SBK_QMATRIX4X4_IDX                                           111
#define SBK_QQUATERNION_IDX                                          161
#define SBK_QVECTOR4D_IDX                                            259
#define SBK_QVECTOR3D_IDX                                            258
#define SBK_QPALETTE_IDX                                             149
#define SBK_QPALETTE_COLORGROUP_IDX                                  150
#define SBK_QPALETTE_COLORROLE_IDX                                   151
#define SBK_QSURFACE_IDX                                             177
#define SBK_QSURFACE_SURFACECLASS_IDX                                178
#define SBK_QSURFACE_SURFACETYPE_IDX                                 179
#define SBK_QSURFACEFORMAT_IDX                                       180
#define SBK_QSURFACEFORMAT_FORMATOPTION_IDX                          181
#define SBK_QFLAGS_QSURFACEFORMAT_FORMATOPTION__IDX                  43
#define SBK_QSURFACEFORMAT_SWAPBEHAVIOR_IDX                          184
#define SBK_QSURFACEFORMAT_RENDERABLETYPE_IDX                        183
#define SBK_QSURFACEFORMAT_OPENGLCONTEXTPROFILE_IDX                  182
#define SBK_QCURSOR_IDX                                              25
#define SBK_QSTANDARDITEM_IDX                                        172
#define SBK_QSTANDARDITEM_ITEMTYPE_IDX                               173
#define SBK_QFONT_IDX                                                51
#define SBK_QFONT_STYLEHINT_IDX                                      56
#define SBK_QFONT_STYLESTRATEGY_IDX                                  57
#define SBK_QFONT_HINTINGPREFERENCE_IDX                              266
#define SBK_QFONT_WEIGHT_IDX                                         58
#define SBK_QFONT_STYLE_IDX                                          55
#define SBK_QFONT_STRETCH_IDX                                        54
#define SBK_QFONT_CAPITALIZATION_IDX                                 52
#define SBK_QFONT_SPACINGTYPE_IDX                                    53
#define SBK_QTEXTCHARFORMAT_IDX                                      194
#define SBK_QTEXTCHARFORMAT_VERTICALALIGNMENT_IDX                    197
#define SBK_QTEXTCHARFORMAT_UNDERLINESTYLE_IDX                       196
#define SBK_QTEXTCHARFORMAT_FONTPROPERTIESINHERITANCEBEHAVIOR_IDX    195
#define SBK_QTEXTIMAGEFORMAT_IDX                                     220
#define SBK_QTEXTTABLECELLFORMAT_IDX                                 244
#define SBK_QGRADIENT_IDX                                            65
#define SBK_QGRADIENT_TYPE_IDX                                       69
#define SBK_QGRADIENT_SPREAD_IDX                                     68
#define SBK_QGRADIENT_COORDINATEMODE_IDX                             66
#define SBK_QGRADIENT_INTERPOLATIONMODE_IDX                          67
#define SBK_QLINEARGRADIENT_IDX                                      101
#define SBK_QCONICALGRADIENT_IDX                                     22
#define SBK_QRADIALGRADIENT_IDX                                      162
#define SBK_QBRUSH_IDX                                               15
#define SBK_QPIXMAPCACHE_IDX                                         156
#define SBK_QPIXMAPCACHE_KEY_IDX                                     157
#define SBK_QPICTUREIO_IDX                                           154
#define SBK_QIMAGEIOHANDLER_IDX                                      84
#define SBK_QIMAGEIOHANDLER_IMAGEOPTION_IDX                          85
#define SBK_QIMAGEIOHANDLER_TRANSFORMATION_IDX                       86
#define SBK_QFLAGS_QIMAGEIOHANDLER_TRANSFORMATION__IDX               36
#define SBK_QTRANSFORM_IDX                                           253
#define SBK_QTRANSFORM_TRANSFORMATIONTYPE_IDX                        254
#define SBK_QPAINTERPATHSTROKER_IDX                                  148
#define SBK_QMATRIX_IDX                                              102
#define SBK_QPAINTERPATH_IDX                                         145
#define SBK_QPAINTERPATH_ELEMENTTYPE_IDX                             147
#define SBK_QPAINTERPATH_ELEMENT_IDX                                 146
#define SBK_QPOLYGONF_IDX                                            159
#define SBK_QVECTOR_QPOINTF_IDX                                      159
#define SBK_QPOLYGON_IDX                                             158
#define SBK_QVECTOR_QPOINT_IDX                                       158
#define SBK_QPAINTDEVICE_IDX                                         131
#define SBK_QPAINTDEVICE_PAINTDEVICEMETRIC_IDX                       132
#define SBK_QPICTURE_IDX                                             153
#define SBK_QPAGEDPAINTDEVICE_IDX                                    128
#define SBK_QPAGEDPAINTDEVICE_PAGESIZE_IDX                           130
#define SBK_QPAGEDPAINTDEVICE_MARGINS_IDX                            129
#define SBK_QACCESSIBLEEVENT_IDX                                     11
#define SBK_QACCESSIBLEINTERFACE_IDX                                 12
#define SBK_QACCESSIBLE_IDX                                          3
#define SBK_QACCESSIBLE_EVENT_IDX                                    4
#define SBK_QACCESSIBLE_ROLE_IDX                                     7
#define SBK_QACCESSIBLE_TEXT_IDX                                     9
#define SBK_QACCESSIBLE_RELATIONFLAG_IDX                             6
#define SBK_QACCESSIBLE_INTERFACETYPE_IDX                            5
#define SBK_QACCESSIBLE_TEXTBOUNDARYTYPE_IDX                         10
#define SBK_QACCESSIBLE_STATE_IDX                                    8
#define SBK_QTOUCHDEVICE_IDX                                         247
#define SBK_QTOUCHDEVICE_DEVICETYPE_IDX                              249
#define SBK_QTOUCHDEVICE_CAPABILITYFLAG_IDX                          248
#define SBK_QFLAGS_QTOUCHDEVICE_CAPABILITYFLAG__IDX                  48
#define SBK_QVECTOR2D_IDX                                            257
#define SBK_QKEYSEQUENCE_IDX                                         97
#define SBK_QKEYSEQUENCE_STANDARDKEY_IDX                             100
#define SBK_QKEYSEQUENCE_SEQUENCEFORMAT_IDX                          98
#define SBK_QKEYSEQUENCE_SEQUENCEMATCH_IDX                           99
#define SBK_QCOLOR_IDX                                               19
#define SBK_QCOLOR_SPEC_IDX                                          21
#define SBK_QCOLOR_NAMEFORMAT_IDX                                    20
#define SBK_QTEXTLAYOUT_IDX                                          224
#define SBK_QTEXTLAYOUT_CURSORMODE_IDX                               225
#define SBK_QTEXTLAYOUT_FORMATRANGE_IDX                              226
#define SBK_QIMAGE_IDX                                               81
#define SBK_QIMAGE_INVERTMODE_IDX                                    83
#define SBK_QIMAGE_FORMAT_IDX                                        82
#define SBK_QPIXMAP_IDX                                              155
#define SBK_QBITMAP_IDX                                              14
#define SBK_QICON_IDX                                                74
#define SBK_QICON_MODE_IDX                                           75
#define SBK_QICON_STATE_IDX                                          76
#define SBK_QICONENGINE_IDX                                          78
#define SBK_QICONENGINE_ICONENGINEHOOK_IDX                           80
#define SBK_QICONENGINE_AVAILABLESIZESARGUMENT_IDX                   79
#define SBK_QPAINTER_IDX                                             140
#define SBK_QPAINTER_RENDERHINT_IDX                                  144
#define SBK_QFLAGS_QPAINTER_RENDERHINT__IDX                          42
#define SBK_QPAINTER_PIXMAPFRAGMENTHINT_IDX                          143
#define SBK_QFLAGS_QPAINTER_PIXMAPFRAGMENTHINT__IDX                  41
#define SBK_QPAINTER_COMPOSITIONMODE_IDX                             141
#define SBK_QPAINTER_PIXMAPFRAGMENT_IDX                              142
#define SBK_QEXPOSEEVENT_IDX                                         34
#define SBK_QICONDRAGEVENT_IDX                                       77
#define SBK_QCLOSEEVENT_IDX                                          18
#define SBK_QHIDEEVENT_IDX                                           72
#define SBK_QSHOWEVENT_IDX                                           171
#define SBK_QFOCUSEVENT_IDX                                          50
#define SBK_QMOVEEVENT_IDX                                           113
#define SBK_QPAINTEVENT_IDX                                          139
#define SBK_QFILEOPENEVENT_IDX                                       35
#define SBK_QACTIONEVENT_IDX                                         13
#define SBK_QINPUTEVENT_IDX                                          91
#define SBK_QMOUSEEVENT_IDX                                          112
#define SBK_QTABLETEVENT_IDX                                         186
#define SBK_QTABLETEVENT_TABLETDEVICE_IDX                            188
#define SBK_QTABLETEVENT_POINTERTYPE_IDX                             187
#define SBK_QWHEELEVENT_IDX                                          261
#define SBK_QKEYEVENT_IDX                                            96
#define SBK_QCONTEXTMENUEVENT_IDX                                    23
#define SBK_QCONTEXTMENUEVENT_REASON_IDX                             24
#define SBK_QTOUCHEVENT_IDX                                          250
#define SBK_QTOUCHEVENT_TOUCHPOINT_IDX                               251
#define SBK_QTOUCHEVENT_TOUCHPOINT_INFOFLAG_IDX                      252
#define SBK_QFLAGS_QTOUCHEVENT_TOUCHPOINT_INFOFLAG__IDX              49
#define SBK_QHOVEREVENT_IDX                                          73
#define SBK_QSHORTCUTEVENT_IDX                                       170
#define SBK_QTOOLBARCHANGEEVENT_IDX                                  246
#define SBK_QWINDOWSTATECHANGEEVENT_IDX                              265
#define SBK_QDROPEVENT_IDX                                           33
#define SBK_QDRAGMOVEEVENT_IDX                                       32
#define SBK_QDRAGENTEREVENT_IDX                                      30
#define SBK_QINPUTMETHODEVENT_IDX                                    92
#define SBK_QINPUTMETHODEVENT_ATTRIBUTETYPE_IDX                      94
#define SBK_QINPUTMETHODEVENT_ATTRIBUTE_IDX                          93
#define SBK_QHELPEVENT_IDX                                           71
#define SBK_QDRAGLEAVEEVENT_IDX                                      31
#define SBK_QWHATSTHISCLICKEDEVENT_IDX                               260
#define SBK_QSTATUSTIPEVENT_IDX                                      175
#define SBK_QRESIZEEVENT_IDX                                         166
#define SBK_QREGION_IDX                                              164
#define SBK_QREGION_REGIONTYPE_IDX                                   165
#define SBK_QOPENGLSHADER_IDX                                        126
#define SBK_QOPENGLSHADER_SHADERTYPEBIT_IDX                          127
#define SBK_QFLAGS_QOPENGLSHADER_SHADERTYPEBIT__IDX                  38
#define SBK_QSYNTAXHIGHLIGHTER_IDX                                   185
#define SBK_QABSTRACTTEXTDOCUMENTLAYOUT_IDX                          0
#define SBK_QABSTRACTTEXTDOCUMENTLAYOUT_PAINTCONTEXT_IDX             1
#define SBK_QABSTRACTTEXTDOCUMENTLAYOUT_SELECTION_IDX                2
#define SBK_QWINDOW_IDX                                              262
#define SBK_QWINDOW_VISIBILITY_IDX                                   264
#define SBK_QWINDOW_ANCESTORMODE_IDX                                 263
#define SBK_QTEXTOBJECT_IDX                                          235
#define SBK_QTEXTBLOCKGROUP_IDX                                      192
#define SBK_QTEXTLIST_IDX                                            232
#define SBK_QTEXTFRAME_IDX                                           215
#define SBK_QTEXTTABLE_IDX                                           242
#define SBK_QTEXTFRAME_ITERATOR_IDX                                  216
#define SBK_QSESSIONMANAGER_IDX                                      168
#define SBK_QSESSIONMANAGER_RESTARTHINT_IDX                          169
#define SBK_QSCREEN_IDX                                              167
#define SBK_QCLIPBOARD_IDX                                           16
#define SBK_QCLIPBOARD_MODE_IDX                                      17
#define SBK_QDRAG_IDX                                                29
#define SBK_QGUIAPPLICATION_IDX                                      70
#define SBK_QOPENGLCONTEXT_IDX                                       122
#define SBK_QOPENGLCONTEXT_OPENGLMODULETYPE_IDX                      123
#define SBK_QSTANDARDITEMMODEL_IDX                                   174
#define SBK_QSTRINGLISTMODEL_IDX                                     176
#define SBK_QVALIDATOR_IDX                                           255
#define SBK_QVALIDATOR_STATE_IDX                                     256
#define SBK_QDOUBLEVALIDATOR_IDX                                     27
#define SBK_QDOUBLEVALIDATOR_NOTATION_IDX                            28
#define SBK_QINTVALIDATOR_IDX                                        95
#define SBK_QREGEXPVALIDATOR_IDX                                     163
#define SBK_QPYTEXTOBJECT_IDX                                        160
#define SBK_QTEXTDOCUMENTFRAGMENT_IDX                                207
#define SBK_QTEXTDOCUMENT_IDX                                        202
#define SBK_QTEXTDOCUMENT_METAINFORMATION_IDX                        204
#define SBK_QTEXTDOCUMENT_FINDFLAG_IDX                               203
#define SBK_QFLAGS_QTEXTDOCUMENT_FINDFLAG__IDX                       44
#define SBK_QTEXTDOCUMENT_RESOURCETYPE_IDX                           205
#define SBK_QTEXTDOCUMENT_STACKS_IDX                                 206
#define SBK_QTEXTDOCUMENTWRITER_IDX                                  208
#define SBK_QIMAGEWRITER_IDX                                         89
#define SBK_QIMAGEWRITER_IMAGEWRITERERROR_IDX                        90
#define SBK_QIMAGEREADER_IDX                                         87
#define SBK_QIMAGEREADER_IMAGEREADERERROR_IDX                        88
#define SBK_QMOVIE_IDX                                               114
#define SBK_QMOVIE_MOVIESTATE_IDX                                    116
#define SBK_QMOVIE_CACHEMODE_IDX                                     115
#define SBK_QtGui_IDX_COUNT                                          268

// This variable stores all Python types exported by this module.
extern PyTypeObject** SbkPySide2_QtGuiTypes;

// This variable stores all type converters exported by this module.
extern SbkConverter** SbkPySide2_QtGuiTypeConverters;

// Converter indices
#define SBK_WID_IDX                                                  0
#define SBK_QTGUI_QVECTOR_QTEXTLAYOUT_FORMATRANGE_IDX                1 // QVector<QTextLayout::FormatRange >
#define SBK_QTGUI_QLIST_INT_IDX                                      2 // QList<int >
#define SBK_QTGUI_QLIST_QFONTDATABASE_WRITINGSYSTEM_IDX              3 // QList<QFontDatabase::WritingSystem >
#define SBK_QTGUI_QVECTOR_QTEXTLENGTH_IDX                            4 // QVector<QTextLength >
#define SBK_QTGUI_QMAP_INT_QVARIANT_IDX                              5 // QMap<int, QVariant >
#define SBK_QTGUI_QLIST_QTEXTOPTION_TAB_IDX                          6 // const QList<QTextOption::Tab > &
#define SBK_QTGUI_QVECTOR_QREAL_IDX                                  7 // QVector<qreal >
#define SBK_QTGUI_QLIST_QREAL_IDX                                    8 // const QList<qreal > &
#define SBK_QTGUI_QVECTOR_QSIZE_IDX                                  9 // QVector<QSize >
#define SBK_QTGUI_QLIST_FLOAT_IDX                                    10 // QList<float >
#define SBK_QTGUI_QPAIR_INT_INT_IDX                                  11 // QPair<int, int >
#define SBK_QTGUI_QLIST_QSTANDARDITEMPTR_IDX                         12 // const QList<QStandardItem * > &
#define SBK_QTGUI_QPAIR_QREAL_QCOLOR_IDX                             13 // QPair<qreal, QColor >
#define SBK_QTGUI_QVECTOR_QPAIR_QREAL_QCOLOR_IDX                     14 // const QVector<QPair<qreal, QColor > > &
#define SBK_QTGUI_QLIST_QBYTEARRAY_IDX                               15 // QList<QByteArray >
#define SBK_QTGUI_QLIST_QPOLYGONF_IDX                                16 // QList<QPolygonF >
#define SBK_QTGUI_QVECTOR_QPOINTF_IDX                                17 // const QVector<QPointF > &
#define SBK_QTGUI_QLIST_QPOINTF_IDX                                  18 // const QList<QPointF > &
#define SBK_QTGUI_QVECTOR_QPOINT_IDX                                 19 // const QVector<QPoint > &
#define SBK_QTGUI_QLIST_QPOINT_IDX                                   20 // const QList<QPoint > &
#define SBK_QTGUI_QLIST_CONSTQTOUCHDEVICEPTR_IDX                     21 // QList<const QTouchDevice * >
#define SBK_QTGUI_QLIST_QKEYSEQUENCE_IDX                             22 // QList<QKeySequence >
#define SBK_QTGUI_QLIST_QTEXTLAYOUT_FORMATRANGE_IDX                  23 // QList<QTextLayout::FormatRange >
#define SBK_QTGUI_QVECTOR_UNSIGNEDINT_IDX                            24 // QVector<unsigned int >
#define SBK_QTGUI_QLIST_QSIZE_IDX                                    25 // QList<QSize >
#define SBK_QTGUI_QVECTOR_QLINE_IDX                                  26 // const QVector<QLine > &
#define SBK_QTGUI_QVECTOR_QLINEF_IDX                                 27 // const QVector<QLineF > &
#define SBK_QTGUI_QVECTOR_QRECT_IDX                                  28 // const QVector<QRect > &
#define SBK_QTGUI_QVECTOR_QRECTF_IDX                                 29 // const QVector<QRectF > &
#define SBK_QTGUI_QLIST_QTOUCHEVENT_TOUCHPOINT_IDX                   30 // const QList<QTouchEvent::TouchPoint > &
#define SBK_QTGUI_QLIST_QINPUTMETHODEVENT_ATTRIBUTE_IDX              31 // const QList<QInputMethodEvent::Attribute > &
#define SBK_QTGUI_QLIST_QOBJECTPTR_IDX                               32 // const QList<QObject * > &
#define SBK_QTGUI_QLIST_QTEXTBLOCK_IDX                               33 // QList<QTextBlock >
#define SBK_QTGUI_QLIST_QTEXTFRAMEPTR_IDX                            34 // QList<QTextFrame * >
#define SBK_QTGUI_QLIST_QSCREENPTR_IDX                               35 // QList<QScreen * >
#define SBK_QTGUI_QLIST_QWINDOWPTR_IDX                               36 // QList<QWindow * >
#define SBK_QTGUI_QSET_QBYTEARRAY_IDX                                37 // QSet<QByteArray >
#define SBK_QTGUI_QVECTOR_INT_IDX                                    38 // const QVector<int > &
#define SBK_QTGUI_QHASH_INT_QBYTEARRAY_IDX                           39 // const QHash<int, QByteArray > &
#define SBK_QTGUI_QLIST_QPERSISTENTMODELINDEX_IDX                    40 // const QList<QPersistentModelIndex > &
#define SBK_QTGUI_QVECTOR_QTEXTFORMAT_IDX                            41 // QVector<QTextFormat >
#define SBK_QTGUI_QLIST_QVARIANT_IDX                                 42 // QList<QVariant >
#define SBK_QTGUI_QLIST_QSTRING_IDX                                  43 // QList<QString >
#define SBK_QTGUI_QMAP_QSTRING_QVARIANT_IDX                          44 // QMap<QString, QVariant >
#define SBK_QtGui_CONVERTERS_IDX_COUNT                               45

// Macros for type check

namespace Shiboken
{

// PyType functions, to get the PyObjectType for a type T
template<> inline PyTypeObject* SbkType< ::QMatrix4x3 >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QMATRIX4X3_IDX]); }
template<> inline PyTypeObject* SbkType< ::QMatrix4x2 >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QMATRIX4X2_IDX]); }
template<> inline PyTypeObject* SbkType< ::QMatrix3x4 >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QMATRIX3X4_IDX]); }
template<> inline PyTypeObject* SbkType< ::QMatrix3x3 >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QMATRIX3X3_IDX]); }
template<> inline PyTypeObject* SbkType< ::QMatrix3x2 >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QMATRIX3X2_IDX]); }
template<> inline PyTypeObject* SbkType< ::QMatrix2x4 >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QMATRIX2X4_IDX]); }
template<> inline PyTypeObject* SbkType< ::QMatrix2x3 >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QMATRIX2X3_IDX]); }
template<> inline PyTypeObject* SbkType< ::QMatrix2x2 >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QMATRIX2X2_IDX]); }
template<> inline PyTypeObject* SbkType< ::QDesktopServices >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QDESKTOPSERVICES_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextTableCell >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTTABLECELL_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextFragment >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTFRAGMENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextBlock >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTBLOCK_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextBlock::iterator >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTBLOCK_ITERATOR_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextBlockUserData >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTBLOCKUSERDATA_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextObjectInterface >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTOBJECTINTERFACE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextLine::Edge >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTLINE_EDGE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextLine::CursorPosition >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTLINE_CURSORPOSITION_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextLine >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTLINE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextInlineObject >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTINLINEOBJECT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextCursor::MoveMode >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTCURSOR_MOVEMODE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextCursor::MoveOperation >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTCURSOR_MOVEOPERATION_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextCursor::SelectionType >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTCURSOR_SELECTIONTYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextCursor >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTCURSOR_IDX]); }
template<> inline PyTypeObject* SbkType< ::QFontDatabase::WritingSystem >() { return SbkPySide2_QtGuiTypes[SBK_QFONTDATABASE_WRITINGSYSTEM_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFontDatabase::SystemFont >() { return SbkPySide2_QtGuiTypes[SBK_QFONTDATABASE_SYSTEMFONT_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFontDatabase >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QFONTDATABASE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextFormat::FormatType >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTFORMAT_FORMATTYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextFormat::Property >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTFORMAT_PROPERTY_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextFormat::ObjectTypes >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTFORMAT_OBJECTTYPES_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextFormat::PageBreakFlag >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTFORMAT_PAGEBREAKFLAG_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QTextFormat::PageBreakFlag> >() { return SbkPySide2_QtGuiTypes[SBK_QFLAGS_QTEXTFORMAT_PAGEBREAKFLAG__IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextFormat >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTFORMAT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextBlockFormat::LineHeightTypes >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTBLOCKFORMAT_LINEHEIGHTTYPES_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextBlockFormat >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTBLOCKFORMAT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextListFormat::Style >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTLISTFORMAT_STYLE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextListFormat >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTLISTFORMAT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextFrameFormat::Position >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTFRAMEFORMAT_POSITION_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextFrameFormat::BorderStyle >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTFRAMEFORMAT_BORDERSTYLE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextFrameFormat >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTFRAMEFORMAT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextTableFormat >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTTABLEFORMAT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextLength::Type >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTLENGTH_TYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextLength >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTLENGTH_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPaintEngineState >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPAINTENGINESTATE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPaintEngine::PaintEngineFeature >() { return SbkPySide2_QtGuiTypes[SBK_QPAINTENGINE_PAINTENGINEFEATURE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QPaintEngine::PaintEngineFeature> >() { return SbkPySide2_QtGuiTypes[SBK_QFLAGS_QPAINTENGINE_PAINTENGINEFEATURE__IDX]; }
template<> inline PyTypeObject* SbkType< ::QPaintEngine::DirtyFlag >() { return SbkPySide2_QtGuiTypes[SBK_QPAINTENGINE_DIRTYFLAG_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QPaintEngine::DirtyFlag> >() { return SbkPySide2_QtGuiTypes[SBK_QFLAGS_QPAINTENGINE_DIRTYFLAG__IDX]; }
template<> inline PyTypeObject* SbkType< ::QPaintEngine::PolygonDrawMode >() { return SbkPySide2_QtGuiTypes[SBK_QPAINTENGINE_POLYGONDRAWMODE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QPaintEngine::Type >() { return SbkPySide2_QtGuiTypes[SBK_QPAINTENGINE_TYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QPaintEngine >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPAINTENGINE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextItem::RenderFlag >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTITEM_RENDERFLAG_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QTextItem::RenderFlag> >() { return SbkPySide2_QtGuiTypes[SBK_QFLAGS_QTEXTITEM_RENDERFLAG__IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextItem >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTITEM_IDX]); }
template<> inline PyTypeObject* SbkType< ::QFontMetricsF >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QFONTMETRICSF_IDX]); }
template<> inline PyTypeObject* SbkType< ::QFontMetrics >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QFONTMETRICS_IDX]); }
template<> inline PyTypeObject* SbkType< ::QFontInfo >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QFONTINFO_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPen >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPEN_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextOption::TabType >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTOPTION_TABTYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextOption::WrapMode >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTOPTION_WRAPMODE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextOption::Flag >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTOPTION_FLAG_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QTextOption::Flag> >() { return SbkPySide2_QtGuiTypes[SBK_QFLAGS_QTEXTOPTION_FLAG__IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextOption >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTOPTION_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextOption::Tab >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTOPTION_TAB_IDX]); }
template<> inline PyTypeObject* SbkType< ::QOpenGLFramebufferObject::Attachment >() { return SbkPySide2_QtGuiTypes[SBK_QOPENGLFRAMEBUFFEROBJECT_ATTACHMENT_IDX]; }
template<> inline PyTypeObject* SbkType< ::QOpenGLFramebufferObject >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QOPENGLFRAMEBUFFEROBJECT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QOpenGLBuffer::Type >() { return SbkPySide2_QtGuiTypes[SBK_QOPENGLBUFFER_TYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QOpenGLBuffer::UsagePattern >() { return SbkPySide2_QtGuiTypes[SBK_QOPENGLBUFFER_USAGEPATTERN_IDX]; }
template<> inline PyTypeObject* SbkType< ::QOpenGLBuffer::Access >() { return SbkPySide2_QtGuiTypes[SBK_QOPENGLBUFFER_ACCESS_IDX]; }
template<> inline PyTypeObject* SbkType< ::QOpenGLBuffer::RangeAccessFlag >() { return SbkPySide2_QtGuiTypes[SBK_QOPENGLBUFFER_RANGEACCESSFLAG_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QOpenGLBuffer::RangeAccessFlag> >() { return SbkPySide2_QtGuiTypes[SBK_QFLAGS_QOPENGLBUFFER_RANGEACCESSFLAG__IDX]; }
template<> inline PyTypeObject* SbkType< ::QOpenGLBuffer >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QOPENGLBUFFER_IDX]); }
template<> inline PyTypeObject* SbkType< ::QMatrix4x4 >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QMATRIX4X4_IDX]); }
template<> inline PyTypeObject* SbkType< ::QQuaternion >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QQUATERNION_IDX]); }
template<> inline PyTypeObject* SbkType< ::QVector4D >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QVECTOR4D_IDX]); }
template<> inline PyTypeObject* SbkType< ::QVector3D >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QVECTOR3D_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPalette::ColorGroup >() { return SbkPySide2_QtGuiTypes[SBK_QPALETTE_COLORGROUP_IDX]; }
template<> inline PyTypeObject* SbkType< ::QPalette::ColorRole >() { return SbkPySide2_QtGuiTypes[SBK_QPALETTE_COLORROLE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QPalette >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPALETTE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QSurface::SurfaceClass >() { return SbkPySide2_QtGuiTypes[SBK_QSURFACE_SURFACECLASS_IDX]; }
template<> inline PyTypeObject* SbkType< ::QSurface::SurfaceType >() { return SbkPySide2_QtGuiTypes[SBK_QSURFACE_SURFACETYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QSurface >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QSURFACE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QSurfaceFormat::FormatOption >() { return SbkPySide2_QtGuiTypes[SBK_QSURFACEFORMAT_FORMATOPTION_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QSurfaceFormat::FormatOption> >() { return SbkPySide2_QtGuiTypes[SBK_QFLAGS_QSURFACEFORMAT_FORMATOPTION__IDX]; }
template<> inline PyTypeObject* SbkType< ::QSurfaceFormat::SwapBehavior >() { return SbkPySide2_QtGuiTypes[SBK_QSURFACEFORMAT_SWAPBEHAVIOR_IDX]; }
template<> inline PyTypeObject* SbkType< ::QSurfaceFormat::RenderableType >() { return SbkPySide2_QtGuiTypes[SBK_QSURFACEFORMAT_RENDERABLETYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QSurfaceFormat::OpenGLContextProfile >() { return SbkPySide2_QtGuiTypes[SBK_QSURFACEFORMAT_OPENGLCONTEXTPROFILE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QSurfaceFormat >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QSURFACEFORMAT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QCursor >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QCURSOR_IDX]); }
template<> inline PyTypeObject* SbkType< ::QStandardItem::ItemType >() { return SbkPySide2_QtGuiTypes[SBK_QSTANDARDITEM_ITEMTYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QStandardItem >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QSTANDARDITEM_IDX]); }
template<> inline PyTypeObject* SbkType< ::QFont::StyleHint >() { return SbkPySide2_QtGuiTypes[SBK_QFONT_STYLEHINT_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFont::StyleStrategy >() { return SbkPySide2_QtGuiTypes[SBK_QFONT_STYLESTRATEGY_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFont::HintingPreference >() { return SbkPySide2_QtGuiTypes[SBK_QFONT_HINTINGPREFERENCE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFont::Weight >() { return SbkPySide2_QtGuiTypes[SBK_QFONT_WEIGHT_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFont::Style >() { return SbkPySide2_QtGuiTypes[SBK_QFONT_STYLE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFont::Stretch >() { return SbkPySide2_QtGuiTypes[SBK_QFONT_STRETCH_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFont::Capitalization >() { return SbkPySide2_QtGuiTypes[SBK_QFONT_CAPITALIZATION_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFont::SpacingType >() { return SbkPySide2_QtGuiTypes[SBK_QFONT_SPACINGTYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFont >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QFONT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextCharFormat::VerticalAlignment >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTCHARFORMAT_VERTICALALIGNMENT_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextCharFormat::UnderlineStyle >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTCHARFORMAT_UNDERLINESTYLE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextCharFormat::FontPropertiesInheritanceBehavior >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTCHARFORMAT_FONTPROPERTIESINHERITANCEBEHAVIOR_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextCharFormat >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTCHARFORMAT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextImageFormat >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTIMAGEFORMAT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextTableCellFormat >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTTABLECELLFORMAT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QGradient::Type >() { return SbkPySide2_QtGuiTypes[SBK_QGRADIENT_TYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QGradient::Spread >() { return SbkPySide2_QtGuiTypes[SBK_QGRADIENT_SPREAD_IDX]; }
template<> inline PyTypeObject* SbkType< ::QGradient::CoordinateMode >() { return SbkPySide2_QtGuiTypes[SBK_QGRADIENT_COORDINATEMODE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QGradient::InterpolationMode >() { return SbkPySide2_QtGuiTypes[SBK_QGRADIENT_INTERPOLATIONMODE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QGradient >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QGRADIENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QLinearGradient >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QLINEARGRADIENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QConicalGradient >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QCONICALGRADIENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QRadialGradient >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QRADIALGRADIENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QBrush >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QBRUSH_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPixmapCache >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPIXMAPCACHE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPixmapCache::Key >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPIXMAPCACHE_KEY_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPictureIO >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPICTUREIO_IDX]); }
template<> inline PyTypeObject* SbkType< ::QImageIOHandler::ImageOption >() { return SbkPySide2_QtGuiTypes[SBK_QIMAGEIOHANDLER_IMAGEOPTION_IDX]; }
template<> inline PyTypeObject* SbkType< ::QImageIOHandler::Transformation >() { return SbkPySide2_QtGuiTypes[SBK_QIMAGEIOHANDLER_TRANSFORMATION_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QImageIOHandler::Transformation> >() { return SbkPySide2_QtGuiTypes[SBK_QFLAGS_QIMAGEIOHANDLER_TRANSFORMATION__IDX]; }
template<> inline PyTypeObject* SbkType< ::QImageIOHandler >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QIMAGEIOHANDLER_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTransform::TransformationType >() { return SbkPySide2_QtGuiTypes[SBK_QTRANSFORM_TRANSFORMATIONTYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTransform >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTRANSFORM_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPainterPathStroker >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPAINTERPATHSTROKER_IDX]); }
template<> inline PyTypeObject* SbkType< ::QMatrix >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QMATRIX_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPainterPath::ElementType >() { return SbkPySide2_QtGuiTypes[SBK_QPAINTERPATH_ELEMENTTYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QPainterPath >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPAINTERPATH_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPainterPath::Element >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPAINTERPATH_ELEMENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPolygonF >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPOLYGONF_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPolygon >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPOLYGON_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPaintDevice::PaintDeviceMetric >() { return SbkPySide2_QtGuiTypes[SBK_QPAINTDEVICE_PAINTDEVICEMETRIC_IDX]; }
template<> inline PyTypeObject* SbkType< ::QPaintDevice >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPAINTDEVICE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPicture >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPICTURE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPagedPaintDevice::PageSize >() { return SbkPySide2_QtGuiTypes[SBK_QPAGEDPAINTDEVICE_PAGESIZE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QPagedPaintDevice >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPAGEDPAINTDEVICE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPagedPaintDevice::Margins >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPAGEDPAINTDEVICE_MARGINS_IDX]); }
template<> inline PyTypeObject* SbkType< ::QAccessibleEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QACCESSIBLEEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QAccessibleInterface >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QACCESSIBLEINTERFACE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QAccessible::Event >() { return SbkPySide2_QtGuiTypes[SBK_QACCESSIBLE_EVENT_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAccessible::Role >() { return SbkPySide2_QtGuiTypes[SBK_QACCESSIBLE_ROLE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAccessible::Text >() { return SbkPySide2_QtGuiTypes[SBK_QACCESSIBLE_TEXT_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAccessible::RelationFlag >() { return SbkPySide2_QtGuiTypes[SBK_QACCESSIBLE_RELATIONFLAG_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAccessible::InterfaceType >() { return SbkPySide2_QtGuiTypes[SBK_QACCESSIBLE_INTERFACETYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAccessible::TextBoundaryType >() { return SbkPySide2_QtGuiTypes[SBK_QACCESSIBLE_TEXTBOUNDARYTYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QAccessible >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QACCESSIBLE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QAccessible::State >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QACCESSIBLE_STATE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTouchDevice::DeviceType >() { return SbkPySide2_QtGuiTypes[SBK_QTOUCHDEVICE_DEVICETYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTouchDevice::CapabilityFlag >() { return SbkPySide2_QtGuiTypes[SBK_QTOUCHDEVICE_CAPABILITYFLAG_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QTouchDevice::CapabilityFlag> >() { return SbkPySide2_QtGuiTypes[SBK_QFLAGS_QTOUCHDEVICE_CAPABILITYFLAG__IDX]; }
template<> inline PyTypeObject* SbkType< ::QTouchDevice >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTOUCHDEVICE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QVector2D >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QVECTOR2D_IDX]); }
template<> inline PyTypeObject* SbkType< ::QKeySequence::StandardKey >() { return SbkPySide2_QtGuiTypes[SBK_QKEYSEQUENCE_STANDARDKEY_IDX]; }
template<> inline PyTypeObject* SbkType< ::QKeySequence::SequenceFormat >() { return SbkPySide2_QtGuiTypes[SBK_QKEYSEQUENCE_SEQUENCEFORMAT_IDX]; }
template<> inline PyTypeObject* SbkType< ::QKeySequence::SequenceMatch >() { return SbkPySide2_QtGuiTypes[SBK_QKEYSEQUENCE_SEQUENCEMATCH_IDX]; }
template<> inline PyTypeObject* SbkType< ::QKeySequence >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QKEYSEQUENCE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QColor::Spec >() { return SbkPySide2_QtGuiTypes[SBK_QCOLOR_SPEC_IDX]; }
template<> inline PyTypeObject* SbkType< ::QColor::NameFormat >() { return SbkPySide2_QtGuiTypes[SBK_QCOLOR_NAMEFORMAT_IDX]; }
template<> inline PyTypeObject* SbkType< ::QColor >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QCOLOR_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextLayout::CursorMode >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTLAYOUT_CURSORMODE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextLayout >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTLAYOUT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextLayout::FormatRange >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTLAYOUT_FORMATRANGE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QImage::InvertMode >() { return SbkPySide2_QtGuiTypes[SBK_QIMAGE_INVERTMODE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QImage::Format >() { return SbkPySide2_QtGuiTypes[SBK_QIMAGE_FORMAT_IDX]; }
template<> inline PyTypeObject* SbkType< ::QImage >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QIMAGE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPixmap >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPIXMAP_IDX]); }
template<> inline PyTypeObject* SbkType< ::QBitmap >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QBITMAP_IDX]); }
template<> inline PyTypeObject* SbkType< ::QIcon::Mode >() { return SbkPySide2_QtGuiTypes[SBK_QICON_MODE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QIcon::State >() { return SbkPySide2_QtGuiTypes[SBK_QICON_STATE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QIcon >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QICON_IDX]); }
template<> inline PyTypeObject* SbkType< ::QIconEngine::IconEngineHook >() { return SbkPySide2_QtGuiTypes[SBK_QICONENGINE_ICONENGINEHOOK_IDX]; }
template<> inline PyTypeObject* SbkType< ::QIconEngine >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QICONENGINE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QIconEngine::AvailableSizesArgument >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QICONENGINE_AVAILABLESIZESARGUMENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPainter::RenderHint >() { return SbkPySide2_QtGuiTypes[SBK_QPAINTER_RENDERHINT_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QPainter::RenderHint> >() { return SbkPySide2_QtGuiTypes[SBK_QFLAGS_QPAINTER_RENDERHINT__IDX]; }
template<> inline PyTypeObject* SbkType< ::QPainter::PixmapFragmentHint >() { return SbkPySide2_QtGuiTypes[SBK_QPAINTER_PIXMAPFRAGMENTHINT_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QPainter::PixmapFragmentHint> >() { return SbkPySide2_QtGuiTypes[SBK_QFLAGS_QPAINTER_PIXMAPFRAGMENTHINT__IDX]; }
template<> inline PyTypeObject* SbkType< ::QPainter::CompositionMode >() { return SbkPySide2_QtGuiTypes[SBK_QPAINTER_COMPOSITIONMODE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QPainter >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPAINTER_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPainter::PixmapFragment >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPAINTER_PIXMAPFRAGMENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QExposeEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QEXPOSEEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QIconDragEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QICONDRAGEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QCloseEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QCLOSEEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QHideEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QHIDEEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QShowEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QSHOWEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QFocusEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QFOCUSEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QMoveEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QMOVEEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPaintEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPAINTEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QFileOpenEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QFILEOPENEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QActionEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QACTIONEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QInputEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QINPUTEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QMouseEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QMOUSEEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTabletEvent::TabletDevice >() { return SbkPySide2_QtGuiTypes[SBK_QTABLETEVENT_TABLETDEVICE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTabletEvent::PointerType >() { return SbkPySide2_QtGuiTypes[SBK_QTABLETEVENT_POINTERTYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTabletEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTABLETEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QWheelEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QWHEELEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QKeyEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QKEYEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QContextMenuEvent::Reason >() { return SbkPySide2_QtGuiTypes[SBK_QCONTEXTMENUEVENT_REASON_IDX]; }
template<> inline PyTypeObject* SbkType< ::QContextMenuEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QCONTEXTMENUEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTouchEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTOUCHEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTouchEvent::TouchPoint::InfoFlag >() { return SbkPySide2_QtGuiTypes[SBK_QTOUCHEVENT_TOUCHPOINT_INFOFLAG_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QTouchEvent::TouchPoint::InfoFlag> >() { return SbkPySide2_QtGuiTypes[SBK_QFLAGS_QTOUCHEVENT_TOUCHPOINT_INFOFLAG__IDX]; }
template<> inline PyTypeObject* SbkType< ::QTouchEvent::TouchPoint >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTOUCHEVENT_TOUCHPOINT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QHoverEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QHOVEREVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QShortcutEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QSHORTCUTEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QToolBarChangeEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTOOLBARCHANGEEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QWindowStateChangeEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QWINDOWSTATECHANGEEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QDropEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QDROPEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QDragMoveEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QDRAGMOVEEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QDragEnterEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QDRAGENTEREVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QInputMethodEvent::AttributeType >() { return SbkPySide2_QtGuiTypes[SBK_QINPUTMETHODEVENT_ATTRIBUTETYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QInputMethodEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QINPUTMETHODEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QInputMethodEvent::Attribute >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QINPUTMETHODEVENT_ATTRIBUTE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QHelpEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QHELPEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QDragLeaveEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QDRAGLEAVEEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QWhatsThisClickedEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QWHATSTHISCLICKEDEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QStatusTipEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QSTATUSTIPEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QResizeEvent >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QRESIZEEVENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QRegion::RegionType >() { return SbkPySide2_QtGuiTypes[SBK_QREGION_REGIONTYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QRegion >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QREGION_IDX]); }
template<> inline PyTypeObject* SbkType< ::QOpenGLShader::ShaderTypeBit >() { return SbkPySide2_QtGuiTypes[SBK_QOPENGLSHADER_SHADERTYPEBIT_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QOpenGLShader::ShaderTypeBit> >() { return SbkPySide2_QtGuiTypes[SBK_QFLAGS_QOPENGLSHADER_SHADERTYPEBIT__IDX]; }
template<> inline PyTypeObject* SbkType< ::QOpenGLShader >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QOPENGLSHADER_IDX]); }
template<> inline PyTypeObject* SbkType< ::QSyntaxHighlighter >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QSYNTAXHIGHLIGHTER_IDX]); }
template<> inline PyTypeObject* SbkType< ::QAbstractTextDocumentLayout >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QABSTRACTTEXTDOCUMENTLAYOUT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QAbstractTextDocumentLayout::PaintContext >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QABSTRACTTEXTDOCUMENTLAYOUT_PAINTCONTEXT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QAbstractTextDocumentLayout::Selection >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QABSTRACTTEXTDOCUMENTLAYOUT_SELECTION_IDX]); }
template<> inline PyTypeObject* SbkType< ::QWindow::Visibility >() { return SbkPySide2_QtGuiTypes[SBK_QWINDOW_VISIBILITY_IDX]; }
template<> inline PyTypeObject* SbkType< ::QWindow::AncestorMode >() { return SbkPySide2_QtGuiTypes[SBK_QWINDOW_ANCESTORMODE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QWindow >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QWINDOW_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextObject >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTOBJECT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextBlockGroup >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTBLOCKGROUP_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextList >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTLIST_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextFrame >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTFRAME_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextTable >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTTABLE_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextFrame::iterator >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTFRAME_ITERATOR_IDX]); }
template<> inline PyTypeObject* SbkType< ::QSessionManager::RestartHint >() { return SbkPySide2_QtGuiTypes[SBK_QSESSIONMANAGER_RESTARTHINT_IDX]; }
template<> inline PyTypeObject* SbkType< ::QSessionManager >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QSESSIONMANAGER_IDX]); }
template<> inline PyTypeObject* SbkType< ::QScreen >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QSCREEN_IDX]); }
template<> inline PyTypeObject* SbkType< ::QClipboard::Mode >() { return SbkPySide2_QtGuiTypes[SBK_QCLIPBOARD_MODE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QClipboard >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QCLIPBOARD_IDX]); }
template<> inline PyTypeObject* SbkType< ::QDrag >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QDRAG_IDX]); }
template<> inline PyTypeObject* SbkType< ::QGuiApplication >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QGUIAPPLICATION_IDX]); }
template<> inline PyTypeObject* SbkType< ::QOpenGLContext::OpenGLModuleType >() { return SbkPySide2_QtGuiTypes[SBK_QOPENGLCONTEXT_OPENGLMODULETYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QOpenGLContext >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QOPENGLCONTEXT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QStandardItemModel >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QSTANDARDITEMMODEL_IDX]); }
template<> inline PyTypeObject* SbkType< ::QStringListModel >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QSTRINGLISTMODEL_IDX]); }
template<> inline PyTypeObject* SbkType< ::QValidator::State >() { return SbkPySide2_QtGuiTypes[SBK_QVALIDATOR_STATE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QValidator >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QVALIDATOR_IDX]); }
template<> inline PyTypeObject* SbkType< ::QDoubleValidator::Notation >() { return SbkPySide2_QtGuiTypes[SBK_QDOUBLEVALIDATOR_NOTATION_IDX]; }
template<> inline PyTypeObject* SbkType< ::QDoubleValidator >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QDOUBLEVALIDATOR_IDX]); }
template<> inline PyTypeObject* SbkType< ::QIntValidator >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QINTVALIDATOR_IDX]); }
template<> inline PyTypeObject* SbkType< ::QRegExpValidator >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QREGEXPVALIDATOR_IDX]); }
template<> inline PyTypeObject* SbkType< ::QPyTextObject >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QPYTEXTOBJECT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextDocumentFragment >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTDOCUMENTFRAGMENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextDocument::MetaInformation >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTDOCUMENT_METAINFORMATION_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextDocument::FindFlag >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTDOCUMENT_FINDFLAG_IDX]; }
template<> inline PyTypeObject* SbkType< ::QFlags<QTextDocument::FindFlag> >() { return SbkPySide2_QtGuiTypes[SBK_QFLAGS_QTEXTDOCUMENT_FINDFLAG__IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextDocument::ResourceType >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTDOCUMENT_RESOURCETYPE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextDocument::Stacks >() { return SbkPySide2_QtGuiTypes[SBK_QTEXTDOCUMENT_STACKS_IDX]; }
template<> inline PyTypeObject* SbkType< ::QTextDocument >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTDOCUMENT_IDX]); }
template<> inline PyTypeObject* SbkType< ::QTextDocumentWriter >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QTEXTDOCUMENTWRITER_IDX]); }
template<> inline PyTypeObject* SbkType< ::QImageWriter::ImageWriterError >() { return SbkPySide2_QtGuiTypes[SBK_QIMAGEWRITER_IMAGEWRITERERROR_IDX]; }
template<> inline PyTypeObject* SbkType< ::QImageWriter >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QIMAGEWRITER_IDX]); }
template<> inline PyTypeObject* SbkType< ::QImageReader::ImageReaderError >() { return SbkPySide2_QtGuiTypes[SBK_QIMAGEREADER_IMAGEREADERERROR_IDX]; }
template<> inline PyTypeObject* SbkType< ::QImageReader >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QIMAGEREADER_IDX]); }
template<> inline PyTypeObject* SbkType< ::QMovie::MovieState >() { return SbkPySide2_QtGuiTypes[SBK_QMOVIE_MOVIESTATE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QMovie::CacheMode >() { return SbkPySide2_QtGuiTypes[SBK_QMOVIE_CACHEMODE_IDX]; }
template<> inline PyTypeObject* SbkType< ::QMovie >() { return reinterpret_cast<PyTypeObject*>(SbkPySide2_QtGuiTypes[SBK_QMOVIE_IDX]); }

} // namespace Shiboken

#endif // SBK_QTGUI_PYTHON_H

