/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2017-2018 Baldur Karlsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#pragma once

#include <QDialog>

namespace Ui
{
class SettingsDialog;
}

class QListWidgetItem;
struct SPIRVDisassembler;

struct ICaptureContext;

class SettingsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit SettingsDialog(ICaptureContext &ctx, QWidget *parent = 0);
  ~SettingsDialog();

  void focusItem(QString item);

private slots:
  // automatic slots

  // global
  void on_pages_itemSelectionChanged();
  void on_okButton_accepted();

  // general
  void on_UIStyle_currentIndexChanged(int index);
  void on_tempDirectory_textEdited(const QString &temp);
  void on_saveDirectory_textEdited(const QString &save);
  void on_browseSaveCaptureDirectory_clicked();
  void on_AllowGlobalHook_toggled(bool checked);
  void on_CheckUpdate_AllowChecks_toggled(bool checked);
  void on_Font_PreferMonospaced_toggled(bool checked);
  void on_AlwaysReplayLocally_toggled(bool checked);
  void on_analyticsAutoSubmit_toggled(bool checked);
  void on_analyticsManualCheck_toggled(bool checked);
  void on_analyticsOptOut_toggled(bool checked);

  // core
  void on_chooseSearchPaths_clicked();
  void on_ExternalTool_RadeonGPUProfiler_textEdited(const QString &rgp);
  void on_browseRGPPath_clicked();

  // texture viewer
  void on_TextureViewer_PerTexSettings_toggled(bool checked);
  void on_TextureViewer_ResetRange_toggled(bool checked);

  // shader viewer
  void on_ShaderViewer_FriendlyNaming_toggled(bool checked);

  void on_addDisasm_clicked();
  void on_deleteDisasm_clicked();
  void on_disassemblers_itemSelectionChanged();
  void on_disassemblers_cellChanged(int row, int column);
  void on_disassemblers_keyPress(QKeyEvent *event);
  void disassemblers_rowMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);

  // event browser
  void on_EventBrowser_TimeUnit_currentIndexChanged(int index);
  void on_EventBrowser_AddFake_toggled(bool checked);
  void on_EventBrowser_HideEmpty_toggled(bool checked);
  void on_EventBrowser_HideAPICalls_toggled(bool checked);
  void on_EventBrowser_ApplyColors_toggled(bool checked);
  void on_EventBrowser_ColorEventRow_toggled(bool checked);

  // comments
  void on_Comments_ShowOnLoad_toggled(bool checked);

  // android
  void on_browseTempCaptureDirectory_clicked();
  void on_browseAndroidSDKPath_clicked();
  void on_browseJDKPath_clicked();
  void on_Android_MaxConnectTimeout_valueChanged(double timeout);
  void on_Android_SDKPath_textEdited(const QString &path);
  void on_Android_JDKPath_textEdited(const QString &path);

  // manual slots
  void formatter_valueChanged(int value);

  void on_analyticsDescribeLabel_linkActivated(const QString &link);

private:
  Ui::SettingsDialog *ui;

  void addDisassembler(const SPIRVDisassembler &disasm);

  ICaptureContext &m_Ctx;
  bool m_Init = false;
  bool m_AddingDisassembler = false;
};
