#include "setupwizard.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include <QButtonGroup>

#include "../osd.h"
#include "../p6vxcommon.h"

// =========================================================================
// SetupWizard
// =========================================================================

SetupWizard::SetupWizard(const QString &defaultRomFolder, QWidget *parent)
	: QWizard(parent)
{
	setWindowTitle(tr("PC6001VX Setup"));
	setWizardStyle(QWizard::ModernStyle);
	setOption(QWizard::NoBackButtonOnStartPage, true);
	// Touch-friendly: no help button, no "what's this" clutter.
	setOption(QWizard::NoDefaultButton, false);

	setPage(Page_Welcome, new WelcomePage(this));
	setPage(Page_RomChoice, new RomChoicePage(this));
	setPage(Page_RomFolder, new RomFolderPage(defaultRomFolder, this));
	setPage(Page_Summary, new SummaryPage(this));

	setStartId(Page_Welcome);
	resize(480, 360);
}

bool SetupWizard::useCompatibleRom() const
{
	auto *choicePage = qobject_cast<RomChoicePage *>(page(Page_RomChoice));
	return choicePage ? choicePage->compatibleRadio->isChecked() : true;
}

QString SetupWizard::selectedRomFolder() const
{
	auto *folderPage = qobject_cast<RomFolderPage *>(page(Page_RomFolder));
	return folderPage ? folderPage->folderEdit->text() : QString();
}

// =========================================================================
// WelcomePage
// =========================================================================

WelcomePage::WelcomePage(QWidget *parent)
	: QWizardPage(parent)
{
	setTitle(tr("Welcome to PC6001VX"));

	auto *label = new QLabel(
		tr("Before you can start emulating, PC6001VX needs a PC-6001 "
		   "system ROM.\n\n"
		   "You can either use the free compatible ROM built into the "
		   "app, or point PC6001VX at your own ROM dump if you have one.\n\n"
		   "Tap Next to choose."));
	label->setWordWrap(true);

	auto *layout = new QVBoxLayout;
	layout->addWidget(label);
	layout->addStretch();
	setLayout(layout);
}

// =========================================================================
// RomChoicePage
// =========================================================================

RomChoicePage::RomChoicePage(QWidget *parent)
	: QWizardPage(parent)
{
	setTitle(tr("Choose a ROM"));

	compatibleRadio = new QRadioButton(tr("Use the built-in compatible ROM (recommended)"));
	customRadio = new QRadioButton(tr("I have my own ROM files"));
	compatibleRadio->setChecked(true);

	auto *group = new QButtonGroup(this);
	group->addButton(compatibleRadio);
	group->addButton(customRadio);

	auto *hint = new QLabel(
		tr("If you don't own real PC-6001 hardware, the compatible ROM "
		   "is the easiest option and works for most software."));
	hint->setWordWrap(true);

	auto *layout = new QVBoxLayout;
	layout->addWidget(compatibleRadio);
	layout->addWidget(customRadio);
	layout->addSpacing(12);
	layout->addWidget(hint);
	layout->addStretch();
	setLayout(layout);

	registerField("useCompatibleRom", compatibleRadio);
}

int RomChoicePage::nextId() const
{
	if (compatibleRadio->isChecked())
		return SetupWizard::Page_Summary;
	return SetupWizard::Page_RomFolder;
}

// =========================================================================
// RomFolderPage
// =========================================================================

RomFolderPage::RomFolderPage(const QString &defaultRomFolder, QWidget *parent)
	: QWizardPage(parent)
{
	setTitle(tr("Select ROM Folder"));

	auto *label = new QLabel(
		tr("Choose the folder that contains your PC-6001 ROM files "
		   "(e.g. BASICROM.60, CGROM60.60)."));
	label->setWordWrap(true);

	folderEdit = new QLineEdit;
	folderEdit->setText(defaultRomFolder);
	folderEdit->setReadOnly(true);
	folderEdit->setPlaceholderText(tr("No folder selected"));

	auto *browseButton = new QPushButton(tr("Browse..."));
	connect(browseButton, &QPushButton::clicked, this, &RomFolderPage::browseForFolder);

	auto *pathLayout = new QHBoxLayout;
	pathLayout->addWidget(folderEdit);
	pathLayout->addWidget(browseButton);

	auto *layout = new QVBoxLayout;
	layout->addWidget(label);
	layout->addLayout(pathLayout);
	layout->addStretch();
	setLayout(layout);

	registerField("romFolder*", folderEdit);
}

void RomFolderPage::browseForFolder()
{
	P6VPATH folder = QSTR2P6VPATH(folderEdit->text());
	OSD_AddDelimiter(folder);
	// Uses the same OSD_FolderDiaog wrapper as the rest of the app, so
	// this correctly goes through Android's Storage Access Framework
	// picker on Android instead of a native desktop dialog.
	OSD_FolderDiaog(nullptr, folder);
	OSD_DelDelimiter(folder);

	if (!folder.empty()) {
		folderEdit->setText(P6VPATH2QSTR(folder));
		emit completeChanged();
	}
}

bool RomFolderPage::isComplete() const
{
	return !folderEdit->text().isEmpty();
}

int RomFolderPage::nextId() const
{
	return SetupWizard::Page_Summary;
}

// =========================================================================
// SummaryPage
// =========================================================================

SummaryPage::SummaryPage(QWidget *parent)
	: QWizardPage(parent)
{
	setTitle(tr("Ready to Go"));
	setFinalPage(true);

	summaryLabel = new QLabel;
	summaryLabel->setWordWrap(true);

	auto *layout = new QVBoxLayout;
	layout->addWidget(summaryLabel);
	layout->addStretch();
	setLayout(layout);
}

void SummaryPage::initializePage()
{
	auto *w = wizard();
	bool compatible = w->field("useCompatibleRom").toBool();

	if (compatible) {
		summaryLabel->setText(
			tr("PC6001VX will use the built-in compatible ROM.\n\n"
			   "Tap Finish to start emulating. You can change this later "
			   "from Settings."));
	} else {
		QString folder = w->field("romFolder").toString();
		summaryLabel->setText(
			tr("PC6001VX will look for ROM files in:\n%1\n\n"
			   "Tap Finish to start emulating. You can change this later "
			   "from Settings.").arg(folder));
	}
}
