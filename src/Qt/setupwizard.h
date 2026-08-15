#ifndef SETUPWIZARD_H
#define SETUPWIZARD_H

#include <QWizard>
#include <QWizardPage>
#include <QString>

class QRadioButton;
class QLineEdit;
class QLabel;

// -----------------------------------------------------------------------
// SetupWizard
//
// Replaces the old chain of blocking QMessageBox / folder-picker dialogs
// that used to run every time PC6001VX couldn't find a valid ROM set.
// Shown once, walks the user through:
//   1. Welcome / explanation
//   2. Choose: built-in compatible ROM, or point at your own ROM folder
//   3. (if "own ROM folder") pick the folder
//   4. Confirmation summary
//
// After QDialog::Accepted, call:
//   wizard.useCompatibleRom()   -> true if built-in ROM was chosen
//   wizard.selectedRomFolder()  -> chosen folder (only valid if !useCompatibleRom())
// -----------------------------------------------------------------------
class SetupWizard : public QWizard
{
	Q_OBJECT

public:
	enum PageId {
		Page_Welcome = 0,
		Page_RomChoice,
		Page_RomFolder,
		Page_Summary
	};

	explicit SetupWizard(const QString &defaultRomFolder, QWidget *parent = nullptr);

	bool useCompatibleRom() const;
	QString selectedRomFolder() const;
};

// ---- Pages -------------------------------------------------------------

class WelcomePage : public QWizardPage
{
	Q_OBJECT
public:
	explicit WelcomePage(QWidget *parent = nullptr);
};

class RomChoicePage : public QWizardPage
{
	Q_OBJECT
public:
	explicit RomChoicePage(QWidget *parent = nullptr);
	int nextId() const override;

private:
	QRadioButton *compatibleRadio;
	QRadioButton *customRadio;

	friend class SetupWizard;
};

class RomFolderPage : public QWizardPage
{
	Q_OBJECT
public:
	explicit RomFolderPage(const QString &defaultRomFolder, QWidget *parent = nullptr);
	bool isComplete() const override;
	int nextId() const override;

private slots:
	void browseForFolder();

private:
	QLineEdit *folderEdit;

	friend class SetupWizard;
};

class SummaryPage : public QWizardPage
{
	Q_OBJECT
public:
	explicit SummaryPage(QWidget *parent = nullptr);
	void initializePage() override;

private:
	QLabel *summaryLabel;
};

#endif // SETUPWIZARD_H
