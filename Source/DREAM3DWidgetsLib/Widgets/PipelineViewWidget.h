/* ============================================================================
 * Copyright (c) 2012 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2012 Dr. Michael A. Groeber (US Air Force Research Laboratories)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Groeber, Michael A. Jackson, the US Air Force,
 * BlueQuartz Software nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  This code was written under United States Air Force Contract number
 *                           FA8650-07-D-5800
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifndef _PipelineViewWidget_H
#define _PipelineViewWidget_H

#include <vector>

#include <QtCore/QTimer>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTableWidget>


#include "DREAM3DLib/Common/PipelineMessage.h"
#include "DREAM3DLib/Common/FilterPipeline.h"

#include "DREAM3DWidgetsLib/DREAM3DWidgetsLib.h"
#include "DREAM3DWidgetsLib/Widgets/PipelineFilterWidget.h"


class QScrollArea;

/*
 *
 */
class DREAM3DWidgetsLib_EXPORT PipelineViewWidget : public QFrame
{
    Q_OBJECT

  public:
    PipelineViewWidget(QWidget* parent = 0);
    virtual ~PipelineViewWidget();

    int filterCount();
    PipelineFilterWidget* filterWidgetAt(int index);
    void clearWidgets();

    /**
     * @brief savePipeline
     * @param filePath
     * @param name
     */
    void savePipeline(const QString& filePath, const QString name, QSettings::Format = QSettings::IniFormat);

    /**
     * @brief getFilterPipeline
     * @return
     */
    FilterPipeline::Pointer getFilterPipeline();


    /**
     * @brief setScrollArea
     * @param sa
     */
    void setScrollArea(QScrollArea* sa);
    /**
     * @brief setAutoScroll
     * @param scroll
     */
    void setAutoScroll(bool scroll) {m_AutoScroll = scroll;}
    /**
     * @brief hasAutoScroll
     * @return
     */
    bool hasAutoScroll() {return m_AutoScroll;}

    /**
     * @brief setAutoScrollMargin
     * @param margin
     */
    void setAutoScrollMargin(int margin) {m_AutoScrollMargin = margin;}

    /**
     * @brief getAutoScrollMargin
     * @return
     */
    int getAutoScrollMargin() {return m_AutoScrollMargin;}

    /**
     * @brief stopAutoScroll Stops the time so the Auto Scrolling will cease.
     */
    void stopAutoScroll();

    /**
     * @brief startAutoScroll Starts the QTimer in charge of auto scrolling the view
     */
    void startAutoScroll();

    /**
     * @brief shouldAutoScroll Figures out if the mouse position is in the margin area that would trigger an
     * autoscroll
     * @param pos
     * @return
     */
    bool shouldAutoScroll(const QPoint& pos);


    void setInputParametersWidget(QWidget* w);



    void newEmptyPipelineViewLayout();
    void resetLayout();

    void setPipelineMessageObserver(QObject* pipelineMessageObserver);

  public slots:
    void addFilter(const QString& filterClassName, int index = -1);
    void addFilterWidget(PipelineFilterWidget *w, AbstractFilter::Pointer filter, int index = -1);

    void removeFilterWidget(PipelineFilterWidget *whoSent);
    void setSelectedFilterWidget(PipelineFilterWidget* w);
    void setFilterBeingDragged(PipelineFilterWidget* w);
    void preflightPipeline();

    /**
     * @brief doAutoScroll This does the actual scrolling of the Widget
     */
    void doAutoScroll();

    void loadPipelineFile(const QString& filePath, QSettings::Format format = QSettings::IniFormat);


  signals:
    void addPlaceHolderFilter(QPoint p);
    void removePlaceHolderFilter();
    void pipelineFileDropped(const QString& file);
    void pipelineHasErrorsSignal();
    void pipelineHasNoErrors();
    void preflightHasMessage(PipelineMessage msg);
    void pipelineIssuesCleared();

  protected:
    void setupGui();
    void dragEnterEvent(QDragEnterEvent* event);
    //  void dragLeaveEvent(QDragLeaveEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent* event);

  private:
    PipelineFilterWidget*            m_SelectedFilterWidget;
    QVBoxLayout*              m_FilterWidgetLayout;
    PipelineFilterWidget*            m_FilterBeingDragged;
    int                       m_DropIndex;
    QLabel*                   m_EmptyPipelineLabel;
    QPoint                    m_LastDragPoint;
    QScrollArea*              m_ScrollArea;
    QTimer                    m_autoScrollTimer;
    bool                      m_AutoScroll;
    int                       m_AutoScrollMargin;
    int                       m_autoScrollCount;
    QWidget*                  m_InputParametersWidget;
    QObject*                  m_PipelineMessageObserver;

    PipelineViewWidget(const PipelineViewWidget&); // Copy Constructor Not Implemented
    void operator=(const PipelineViewWidget&); // Operator '=' Not Implemented
};

#endif /* _PipelineViewWidget_H */

