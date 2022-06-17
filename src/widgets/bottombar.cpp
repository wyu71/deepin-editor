/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QLabel>
#include <QPainter>
#include <QHBoxLayout>
#include <DMenu>
#include <DVerticalLine>
#include "bottombar.h"
#include "../common/utils.h"
#include "../editor/editwrapper.h"
#include "../widgets/window.h"
#include "../editor/replaceallcommond.h"

BottomBar::BottomBar(QWidget *parent)
    : QWidget(parent),
      m_pWrapper(static_cast<EditWrapper *>(parent)),
      m_pPositionLabel(new DLabel),
      m_pCharCountLabel(new DLabel),
      m_pCursorStatus(new DLabel),
      m_pEncodeMenu(DDropdownMenu::createEncodeMenu()),
      m_pHighlightMenu(DDropdownMenu::createHighLightMenu()),
      m_rowStr(tr("Row")),
      m_columnStr(tr("Column")),
      m_chrCountStr(tr("Characters %1")),
      m_scaleLabel(new DLabel),
      m_progressLabel(new DLabel),
      m_progressBar(new DProgressBar)
{
    QFont font;
    font.setFamily("SourceHanSansSC-Normal");
    m_pPositionLabel->setFont(font);
    m_pCharCountLabel->setFont(font);
    m_pCursorStatus->setFont(font);
    m_scaleLabel->setFont(font);
    m_progressLabel->setFont(font);
    m_progressLabel->setText(tr("text loading:"));
    m_progressBar->setRange(0,100);
    m_progressBar->setTextVisible(false);
    m_progressBar->setMinimumWidth(80);
    QHBoxLayout* progressLayout = new QHBoxLayout;
    progressLayout->addWidget(m_progressLabel);
    progressLayout->addWidget(m_progressBar);
    progressLayout->addStretch();

    DFontSizeManager::instance()->bind(m_pPositionLabel, DFontSizeManager::T9);
    DFontSizeManager::instance()->bind(m_pCharCountLabel, DFontSizeManager::T9);
    DFontSizeManager::instance()->bind(m_pCursorStatus, DFontSizeManager::T9);
    DFontSizeManager::instance()->bind(m_scaleLabel, DFontSizeManager::T9);
    DFontSizeManager::instance()->bind(m_progressLabel, DFontSizeManager::T9);

    initFormatMenu();

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(29, 1, 10, 0);
    layout->addLayout(progressLayout);
    layout->addStretch();
    layout->addWidget(m_pPositionLabel);
    layout->addStretch();
    layout->addSpacerItem(new QSpacerItem(110,20,QSizePolicy::Expanding,QSizePolicy::Fixed));
    layout->addWidget(m_pCharCountLabel);

    m_progressBar->hide();
    m_progressLabel->hide();

    m_pCursorStatus->setText(qApp->translate("EditWrapper", "INSERT"));
    m_pPositionLabel->setText(QString("%1 %2  %3 %4").arg(m_rowStr, "1",m_columnStr, "1"));

    m_pCharCountLabel->setText(m_chrCountStr.arg("0"));
    m_pHighlightMenu->setCurrentTextOnly(qApp->translate("TextEdit", "None"));
    m_pEncodeMenu->setCurrentTextOnly(QString("UTF-8"));

    DVerticalLine *pVerticalLine1 = new DVerticalLine();
    DVerticalLine *pVerticalLine2 = new DVerticalLine();
    pVerticalLine1->setFixedSize(1, 15);
    pVerticalLine2->setFixedSize(1, 15);
    DVerticalLine *pVerticalLine3 = new DVerticalLine();
    pVerticalLine3->setFixedSize(1, 15);

    layout->addStretch();
    layout->addWidget(m_scaleLabel);
    layout->addStretch();
    layout->addWidget(m_pCursorStatus);
    layout->addSpacing(10);
    layout->addWidget(pVerticalLine1);
    layout->addWidget(m_pEncodeMenu);
    layout->addWidget(pVerticalLine2);
    layout->addWidget(m_formatMenu);
    layout->addWidget(pVerticalLine3);
    layout->addWidget(m_pHighlightMenu);
    setFixedHeight(32);

    //切换编码
    connect(m_pEncodeMenu, &DDropdownMenu::currentActionChanged, this,[this](QAction* pAct){
        if(!m_pWrapper->getFileLoading() && m_pWrapper->reloadFileEncode(pAct->text().toLocal8Bit())) {
            m_pEncodeMenu->setCurrentTextOnly(pAct->text());
        }
        //先屏蔽，双字节空字符先按照显示字符编码号处理
        //m_pWrapper->clearDoubleCharaterEncode();
    });

    //切换文件类型
    connect(m_pHighlightMenu, &DDropdownMenu::currentActionChanged, this,[this](QAction* pAct) {
        m_pHighlightMenu->setCurrentTextOnly(pAct->text());
    });

    //编码按钮/文本类型按钮失去焦点后，设置光标回到文本框里
    connect(m_pEncodeMenu, &DDropdownMenu::sigSetTextEditFocus, this, &BottomBar::slotSetTextEditFocus);
    connect(m_pHighlightMenu, &DDropdownMenu::sigSetTextEditFocus, this, &BottomBar::slotSetTextEditFocus);
}

BottomBar::~BottomBar()
{
    if (m_pEncodeMenu != nullptr) {
        delete m_pEncodeMenu;
        m_pEncodeMenu = nullptr;
    }

    if (m_pHighlightMenu != nullptr) {
        delete m_pHighlightMenu;
        m_pHighlightMenu = nullptr;
    }
}

void BottomBar::updatePosition(int row, int column)
{
    m_pPositionLabel->setText(QString("%1 %2  %3 %4").arg(m_rowStr, QString::number(row),
                                                          m_columnStr, QString::number(column)));
}

void BottomBar::updateWordCount(int charactorCount)
{
    m_pCharCountLabel->setText(m_chrCountStr.arg(QString::number(charactorCount-1)));
}

void BottomBar::setEncodeName(const QString &name)
{
    m_pEncodeMenu->setCurrentTextOnly(name);
}

void BottomBar::setCursorStatus(const QString &text)
{
    m_pCursorStatus->setText(text);
}

void BottomBar::setPalette(const QPalette &palette)
{
    DPalette paPositionLabel  = DApplicationHelper::instance()->applicationPalette();
    DPalette paCharCountLabel = DApplicationHelper::instance()->applicationPalette();
    DPalette paCursorStatus = DApplicationHelper::instance()->applicationPalette();
    DPalette paEncodeMenu = DApplicationHelper::instance()->applicationPalette();
    DPalette paHighlightMenu = DApplicationHelper::instance()->applicationPalette();

    QColor colorFont = paPositionLabel.textTips().color();

    paPositionLabel.setColor(DPalette::WindowText, colorFont);
    paCharCountLabel.setColor(DPalette::WindowText, colorFont);
    paCursorStatus.setColor(DPalette::WindowText, colorFont);
    paEncodeMenu.setColor(DPalette::WindowText, colorFont);
    paHighlightMenu.setColor(DPalette::WindowText, colorFont);


    m_pPositionLabel->setPalette(paPositionLabel);
    m_pCharCountLabel->setPalette(paCharCountLabel);
    m_pCursorStatus->setPalette(paCursorStatus);
    m_pEncodeMenu->getButton()->setPalette(paEncodeMenu);
    m_pHighlightMenu->getButton()->setPalette(paHighlightMenu);
    m_scaleLabel->setPalette(paPositionLabel);
    m_formatMenu->getButton()->setPalette(paEncodeMenu);

    QString theme = (palette.color(QPalette::Background).lightness() < 128) ? "dark" : "light";
    m_pEncodeMenu->setTheme(theme);
    m_pHighlightMenu->setTheme(theme);
    m_formatMenu->setTheme(theme);

    QWidget::setPalette(palette);
}

void BottomBar::updateSize(int size, bool bIsFindOrReplace)
{
    setFixedHeight(size);
    m_bIsFindOrReplace = bIsFindOrReplace;
}

void BottomBar::setChildEnabled(bool enabled)
{
    m_pEncodeMenu->setEnabled(enabled);
    m_pHighlightMenu->setEnabled(enabled);
    m_pEncodeMenu->setRequestMenu(enabled);
    m_pHighlightMenu->setRequestMenu(enabled);
}

void BottomBar::setChildrenFocus(bool ok,QWidget* preOrderWidget)
{
    m_pEncodeMenu->setChildrenFocus(ok);
    m_pHighlightMenu->setChildrenFocus(ok);
    if(ok) {
        if(preOrderWidget) setTabOrder(preOrderWidget,m_pEncodeMenu->getButton());
        setTabOrder(m_pEncodeMenu->getButton(),m_pHighlightMenu->getButton());
    }
}

void BottomBar::setScaleLabelText(int fontSize)
{
    int maxFont = 50;
    int minFont = 8;
    int midFont = 12;
    QString text;
    if(fontSize == midFont){
        text = "100%";
    }
    else if(fontSize > midFont){
        float delta = (500-100)*1.0/(maxFont - midFont);
        int target = 100 + delta * (fontSize-midFont);
        text = QString("%1%").arg(target);
    }
    else {
        float delta = (100-10)*1.0/(midFont - minFont);
        int target = 100 + delta * (fontSize-midFont);
        text = QString("%1%").arg(target);
    }

    m_scaleLabel->setText(text);
}

void BottomBar::setProgress(int progress)
{
    if(progress<0){
        return;
    }
    m_progressBar->show();
    m_progressLabel->show();
    m_progressBar->setValue(progress);
    if(progress >= 100){
        m_progressBar->hide();
        m_progressLabel->hide();
    }
}

DDropdownMenu *BottomBar::getEncodeMenu()
{
    return m_pEncodeMenu;
}

DDropdownMenu *BottomBar::getHighlightMenu()
{
    return m_pHighlightMenu;
}

void BottomBar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setOpacity(1);

    QColor backgroundColor = palette().color(QPalette::Background);
    QColor bottombarBackgroundColor;
    if (backgroundColor.lightness() < 128) {
        bottombarBackgroundColor = palette().base().color();
        if (bottombarBackgroundColor.name() != "#202020") {
            bottombarBackgroundColor = QColor("#202020");
        }
        bottombarBackgroundColor.setAlphaF(0.7);

    } else {
        bottombarBackgroundColor = palette().base().color();
        if (bottombarBackgroundColor.name() != "#ffffff") {
            bottombarBackgroundColor = QColor("#ffffff");
        }

        bottombarBackgroundColor.setAlphaF(0.7);
    }

    QPainterPath path;
    path.addRect(rect());
    painter.fillPath(path, bottombarBackgroundColor);

    QColor splitLineColor;
    if (backgroundColor.lightness() < 128) {
        splitLineColor = palette().brightText().color();
        if (splitLineColor.name() != "#ffffff") {
            splitLineColor = QColor("#ffffff");
        }
        splitLineColor.setAlphaF(0.5);
    } else {
        splitLineColor = palette().brightText().color();
        if (splitLineColor.name() != "#000000") {
            splitLineColor = QColor("#000000");
        }
        splitLineColor.setAlphaF(0.5);
    }

    if (!m_bIsFindOrReplace) {
	    QPainterPath framePath;
	    framePath.addRect(QRect(rect().x(), rect().y(), rect().width(), 1));
	    painter.setOpacity(0.1);
	    painter.fillPath(framePath, splitLineColor);
    }
}

void BottomBar::slotSetTextEditFocus()
{
    Window *pWindow = static_cast<Window *>(m_pWrapper->window());
    emit pWindow->pressEsc();
}

BottomBar::EndlineFormat BottomBar::getEndlineFormat(const QByteArray& text)
{
    for(int i=0;i<text.size();i++){
        if(text[i]=='\n'){
            return EndlineFormat::Unix;
        }
        if(text[i]=='\r' && i+1<text.size() && text[i+1]=='\n'){
            return EndlineFormat::Windows;
        }
    }

    return EndlineFormat::Unknow;
}

BottomBar::EndlineFormat BottomBar:: getEndlineFormat()
{
    return m_endlineFormat;
}

//初始化行尾格式相关
void BottomBar::initFormatMenu()
{
    m_formatMenu = new DDropdownMenu(this);
    m_formatMenu->setCurrentTextOnly(tr("Unix-Format"));
    DMenu *menu = new DMenu(this);
    QActionGroup* actionGroup = new QActionGroup(menu);
    actionGroup->setExclusive(true);
    m_formatMenu->setMenu(menu);
    m_formatMenu->setMenuActionGroup(actionGroup);

    m_unixAction = menu->addAction(tr("Unix-Format"));
    m_windowsAction = menu->addAction(tr("Windows-Format"));
    m_unixAction->setProperty(FormatActionType,EndlineFormat::Unix);
    m_windowsAction->setProperty(FormatActionType,EndlineFormat::Windows);
    actionGroup->addAction(m_unixAction);
    actionGroup->addAction(m_windowsAction);
    connect(actionGroup, &QActionGroup::triggered, this,&BottomBar::onFormatMenuTrigged);
}

//行尾格式action槽函数
void BottomBar::onFormatMenuTrigged(QAction* action)
{
    if(!action){
        return;
    }
    int type = action->property(FormatActionType).toInt();
    if(m_endlineFormat == type){
        return;
    }

    m_pWrapper->textEditor()->onEndlineFormatChanged(m_endlineFormat,(EndlineFormat)type);
    m_endlineFormat = (EndlineFormat)type;

}

//设置行尾menu text
void BottomBar::setEndlineMenuText(EndlineFormat format)
{
    if(format == EndlineFormat::Unix || format == EndlineFormat::Unknow){
        m_formatMenu->setCurrentTextOnly(tr("Unix-Format"));
        m_endlineFormat = EndlineFormat::Unix;
        m_windowsAction->setCheckable(false);
        m_unixAction->setCheckable(true);
        m_unixAction->setChecked(true);

    }
    else {
        m_formatMenu->setCurrentTextOnly(tr("Windows-Format"));
        m_endlineFormat = EndlineFormat::Windows;
        m_unixAction->setCheckable(false);
        m_windowsAction->setCheckable(true);
        m_windowsAction->setChecked(true);
    }
}
