/*
 *    Copyright 2012, 2013 Jan Dalheimer
 *
 *    This file is part of OpenOrienteering.
 *
 *    OpenOrienteering is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    OpenOrienteering is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with OpenOrienteering.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _OPENORIENTEERING_SETTINGS_DIALOG_PRIVATE_H_
#define _OPENORIENTEERING_SETTINGS_DIALOG_PRIVATE_H_

#include <QHash>
#include <QDialog>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QTabWidget;
class QAbstractButton;
class QComboBox;
class QCheckBox;
class QLabel;
class QSpinBox;
class QGridLayout;
class QVBoxLayout;
QT_END_NAMESPACE

class MainWindow;
class QDialogButtonBox;

class SettingsPage : public QWidget
{
Q_OBJECT
public:
	SettingsPage(QWidget* parent = 0) : QWidget(parent) {}
	virtual void cancel() { changes.clear(); }
	virtual void apply();
	virtual void ok() { this->apply(); }
	virtual QString title() = 0;

protected:
	// The changes to be done when accepted
	QHash<QString, QVariant> changes;
};

class EditorPage : public SettingsPage
{
Q_OBJECT
public:
	EditorPage(QWidget* parent = 0);

	virtual QString title() { return tr("Editor"); }

private slots:
	void antialiasingClicked(bool checked);
	void textAntialiasingClicked(bool checked);
	void toleranceChanged(int value);
	void snapDistanceChanged(int value);
	void fixedAngleSteppingChanged(int value);
	void selectSymbolOfObjectsClicked(bool checked);
	void zoomOutAwayFromCursorClicked(bool checked);
	void drawLastPointOnRightClickClicked(bool checked);
	
	void editToolDeleteBezierPointActionChanged(int index);
	void editToolDeleteBezierPointActionAlternativeChanged(int index);
	
	void rectangleHelperCrossRadiusChanged(int value);
	void rectanglePreviewLineWidthChanged(bool checked);
	
	void keepSettingsOfClosedTemplatesClicked(bool checked);
	
private:
	void updateWidgets();
	
	QCheckBox* antialiasing;
	QCheckBox* text_antialiasing;
	QComboBox* edit_tool_delete_bezier_point_action;
	QComboBox* edit_tool_delete_bezier_point_action_alternative;
};

/*class PrintingPage : public SettingsPage
{
Q_OBJECT
public:
	PrintingPage(QWidget* parent = 0);
	
	virtual QString title() { return tr("Printing"); }
	
private slots:
	
};*/

class GeneralPage : public SettingsPage
{
Q_OBJECT
public:
	GeneralPage(QWidget* parent = 0);

	virtual void apply();
	virtual QString title() { return tr("General"); }

private slots:
	void languageChanged(int index);
	
	void openMRUFileClicked(bool state);
	
	void tipsVisibleClicked(bool state);
	
	void openTranslationFileDialog();
	
private:
	/** Adds the available languages to the language combo box,
	 *  and sets the current element.
	 */
	void updateLanguageBox();
	
	QComboBox* language_box;
	const static int TranslationFromFile;
};

#endif