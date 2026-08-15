#include "gamelibrarydialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QPushButton>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

// =========================================================================
// GameLibraryDialog
//
// Visual language borrowed from DuckStation / NetherSX2's Android UIs:
// near-black background, a scrollable grid of rounded cards, an accent
// color used sparingly, and a lightweight top bar instead of nested
// menus. Real cover art isn't available for PC-6001 software, so each
// card shows the filename + extension badge instead of box art -- same
// layout, no artwork.
// =========================================================================

namespace {
const char *kDarkStyleSheet = R"(
QDialog {
	background-color: #121016;
}
QLabel#titleLabel {
	color: #f0eef5;
	font-size: 20px;
	font-weight: 600;
}
QLabel#subtitleLabel {
	color: #8a8794;
	font-size: 12px;
}
QLabel#emptyLabel {
	color: #8a8794;
	font-size: 13px;
}
QListWidget#gameGrid {
	background-color: #121016;
	border: none;
	outline: none;
}
QListWidget#gameGrid::item {
	background-color: #1e1c24;
	border-radius: 10px;
	margin: 8px;
	padding: 10px;
	color: #e6e4ec;
}
QListWidget#gameGrid::item:selected {
	background-color: #2c2836;
	border: 2px solid #7c5cff;
}
QListWidget#gameGrid::item:hover {
	background-color: #262330;
}
QPushButton {
	background-color: #1e1c24;
	color: #e6e4ec;
	border: none;
	border-radius: 8px;
	padding: 8px 18px;
	font-size: 13px;
}
QPushButton:hover {
	background-color: #2c2836;
}
QPushButton#skipButton {
	background-color: transparent;
	color: #8a8794;
}
QPushButton#rescanButton {
	background-color: #7c5cff;
	color: #ffffff;
}
)";

// Supported PC6001VX software file extensions (matches the drag & drop
// routing in src/p6el.cpp: EXT_P6RAW / EXT_CAS / EXT_P6T / EXT_DISK).
const QStringList kSupportedExtensions = {
	"p6", "cas", "p6t", "d88"
};
} // namespace

GameLibraryDialog::GameLibraryDialog(const QString &tapeFolder, const QString &diskFolder,
									  QWidget *parent)
	: QDialog(parent)
	, tapeDir(tapeFolder)
	, diskDir(diskFolder)
{
	setWindowTitle(tr("PC6001VX"));
	resize(720, 480);

	applyDarkTheme();

	auto *titleLabel = new QLabel(tr("Your Games"));
	titleLabel->setObjectName("titleLabel");

	auto *subtitleLabel = new QLabel(tr("Tap a game to play"));
	subtitleLabel->setObjectName("subtitleLabel");

	auto *titleBox = new QVBoxLayout;
	titleBox->addWidget(titleLabel);
	titleBox->addWidget(subtitleLabel);
	titleBox->setSpacing(2);

	auto *rescanButton = new QPushButton(tr("Rescan"));
	rescanButton->setObjectName("rescanButton");
	connect(rescanButton, &QPushButton::clicked, this, &GameLibraryDialog::rescan);

	auto *topBar = new QHBoxLayout;
	topBar->addLayout(titleBox);
	topBar->addStretch();
	topBar->addWidget(rescanButton);

	grid = new QListWidget;
	grid->setObjectName("gameGrid");
	grid->setViewMode(QListView::IconMode);
	grid->setResizeMode(QListView::Adjust);
	grid->setMovement(QListView::Static);
	grid->setSpacing(8);
	grid->setIconSize(QSize(64, 64));
	grid->setGridSize(QSize(160, 120));
	grid->setSelectionMode(QAbstractItemView::SingleSelection);
	connect(grid, &QListWidget::itemActivated, this, &GameLibraryDialog::onItemActivated);
	// Single tap should launch immediately on a touch device.
	connect(grid, &QListWidget::itemClicked, this, &GameLibraryDialog::onItemActivated);

	emptyLabel = new QLabel(
		tr("No games found.\n\nPut .p6 / .cas / .p6t (tape) or .d88 (disk) files in your "
		   "configured TAPE/DISK folders, then tap Rescan."));
	emptyLabel->setObjectName("emptyLabel");
	emptyLabel->setAlignment(Qt::AlignCenter);
	emptyLabel->setWordWrap(true);

	auto *skipButton = new QPushButton(tr("Skip \u2014 start without a game"));
	skipButton->setObjectName("skipButton");
	connect(skipButton, &QPushButton::clicked, this, &QDialog::reject);

	auto *bottomBar = new QHBoxLayout;
	bottomBar->addStretch();
	bottomBar->addWidget(skipButton);

	auto *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(topBar);
	mainLayout->addWidget(grid, 1);
	mainLayout->addWidget(emptyLabel, 1);
	mainLayout->addLayout(bottomBar);
	setLayout(mainLayout);

	populate();
}

void GameLibraryDialog::applyDarkTheme()
{
	setStyleSheet(QString::fromUtf8(kDarkStyleSheet));
}

void GameLibraryDialog::populate()
{
	grid->clear();

	QStringList folders;
	if (!tapeDir.isEmpty()) folders << tapeDir;
	if (!diskDir.isEmpty() && diskDir != tapeDir) folders << diskDir;

	QStringList nameFilters;
	for (const auto &ext : kSupportedExtensions)
		nameFilters << ("*." + ext);

	for (const auto &folderPath : folders) {
		QDir dir(folderPath);
		if (!dir.exists())
			continue;

		const auto entries = dir.entryInfoList(nameFilters, QDir::Files, QDir::Name);
		for (const auto &info : entries) {
			auto *item = new QListWidgetItem(info.fileName());
			item->setData(Qt::UserRole, info.absoluteFilePath());
			item->setToolTip(info.absoluteFilePath());
			item->setTextAlignment(Qt::AlignCenter);
			grid->addItem(item);
		}
	}

	bool hasGames = grid->count() > 0;
	grid->setVisible(hasGames);
	emptyLabel->setVisible(!hasGames);
}

void GameLibraryDialog::rescan()
{
	populate();
}

void GameLibraryDialog::onItemActivated(QListWidgetItem *item)
{
	if (!item)
		return;
	selectedPath = item->data(Qt::UserRole).toString();
	accept();
}

QString GameLibraryDialog::selectedFilePath() const
{
	return selectedPath;
}
