/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Dr. Michael A. Groeber (US Air Force Research Laboratories
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
#ifndef SGLOGNORMALITEMDELEGATE_H_
#define SGLOGNORMALITEMDELEGATE_H_

#include <iostream>

#include <QtCore/QModelIndex>
#include <QtGui/QComboBox>
#include <QtGui/QPainter>
#include <QtGui/QStyleOptionViewItemV4>
#include <QtGui/QLineEdit>
#include <QtGui/QDoubleValidator>
#include <QtGui/QStyledItemDelegate>

#include "DREAM3DLib/Common/StatsGen.h"
#include "QtSupport/ColorComboPicker.h"
#include "StatsGenerator/TableModels/SGLogNormalTableModel.h"

/**
 * @class SGLogNormalItemDelegate SGLogNormalItemDelegate.h StatsGenerator/SGLogNormalItemDelegate.h
 * @brief This class creates the appropriate Editor Widget for the Tables
 * @author Michael A. Jackson for BlueQuartz Software
 * @date Dec 28, 2010
 * @version 1.0
 */
class SGLogNormalItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

  public:
    explicit SGLogNormalItemDelegate(QObject *parent = 0) :
      QStyledItemDelegate(parent)
    {
    }

    // -----------------------------------------------------------------------------
    //
    // -----------------------------------------------------------------------------
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
      QStyledItemDelegate::paint(painter, option, index);
    }

    // -----------------------------------------------------------------------------
    //
    // -----------------------------------------------------------------------------
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
      QLineEdit* avg;
      QLineEdit* stdDev;
      QDoubleValidator* avgValidator;
      QDoubleValidator* stdDevValidator;
      QComboBox* colorCombo;

      qint32 col = index.column();
      switch(col)
      {
        case SGLogNormalTableModel::BinNumber:
          return NULL;
          break;

        case SGLogNormalTableModel::Average:
          avg = new QLineEdit(parent);
          avg->setFrame(false);
          avgValidator = new QDoubleValidator(avg);
          avgValidator->setDecimals(6);
          avg->setValidator(avgValidator);
          return avg;
        case SGLogNormalTableModel::StdDev:
          stdDev = new QLineEdit(parent);
          stdDev->setFrame(false);
          stdDevValidator = new QDoubleValidator(stdDev);
          stdDevValidator->setDecimals(6);
          stdDev->setValidator(stdDevValidator);
          return stdDev;
        case SGLogNormalTableModel::LineColor:
          colorCombo = new ColorComboPicker(parent);
          return colorCombo;
        default:
          break;
      }
      return QStyledItemDelegate::createEditor(parent, option, index);
    }

    // -----------------------------------------------------------------------------
    //
    // -----------------------------------------------------------------------------
    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
      qint32 col = index.column();
    //  bool ok = false;
      if (col == SGLogNormalTableModel::Average || col == SGLogNormalTableModel::StdDev)
      {
        //     double value = index.model()->data(index).toFloat(&ok);
        QLineEdit* lineEdit = qobject_cast<QLineEdit* > (editor);
        Q_ASSERT(lineEdit);
        lineEdit->setText(index.model()->data(index).toString());
      }
      else if (col == SGLogNormalTableModel::LineColor)
      {
        QString state = index.model()->data(index).toString();
        ColorComboPicker* comboBox = qobject_cast<ColorComboPicker* > (editor);
        Q_ASSERT(comboBox);
        comboBox->setCurrentIndex(comboBox->findText(state));
      }
      else QStyledItemDelegate::setEditorData(editor, index);
    }

    // -----------------------------------------------------------------------------
    //
    // -----------------------------------------------------------------------------
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
      //  std::cout << "SGLogNormalItemDelegate::setModelData" << std::endl;
      qint32 col = index.column();
      //  bool ok = false;
      if (col == SGLogNormalTableModel::Average || col == SGLogNormalTableModel::StdDev)
      {
        QLineEdit* lineEdit = qobject_cast<QLineEdit* > (editor);
        Q_ASSERT(lineEdit);
        bool ok = false;
        double v = lineEdit->text().toFloat(&ok);
        model->setData(index, v);
      }
      else if (col == SGLogNormalTableModel::LineColor)
      {
        ColorComboPicker *comboBox = qobject_cast<ColorComboPicker* > (editor);
        Q_ASSERT(comboBox);
        model->setData(index, comboBox->currentText());
      }
      else QStyledItemDelegate::setModelData(editor, model, index);
    }

  private:
    QModelIndex m_Index;
//    QWidget* m_Widget;
//    QAbstractItemModel* m_Model;

};

#endif /* SGLOGNORMALITEMDELEGATE_H_ */
