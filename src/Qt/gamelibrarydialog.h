#ifndef GAMELIBRARYDIALOG_H
#define GAMELIBRARYDIALOG_H

#include <QDialog>
#include <QString>

class QListWidget;
class QListWidgetItem;
class QLabel;

// -----------------------------------------------------------------------
// GameLibraryDialog
//
// A dark, card-grid game picker in the style of DuckStation/NetherSX2's
// Android launchers: scans the configured TAPE and DISK folders for
// supported game files and shows them as a tappable grid. Tapping a
// game accepts the dialog; selectedFilePath() then holds the file to
// boot straight into. "Skip" starts the emulator with nothing inserted
// (old behavior).
// -----------------------------------------------------------------------
class GameLibraryDialog : public QDialog
{
	Q_OBJECT

public:
	explicit GameLibraryDialog(const QString &tapeFolder, const QString &diskFolder,
								QWidget *parent = nullptr);

	// Valid only if the dialog was accepted via a game selection
	// (not the Skip/Settings buttons).
	QString selectedFilePath() const;

private slots:
	void onItemActivated(QListWidgetItem *item);
	void rescan();

private:
	void applyDarkTheme();
	void populate();

	QString tapeDir;
	QString diskDir;
	QListWidget *grid;
	QLabel *emptyLabel;
	QString selectedPath;
};

#endif // GAMELIBRARYDIALOG_H
