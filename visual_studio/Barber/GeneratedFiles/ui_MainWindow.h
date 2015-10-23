/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindowClass
{
public:
    QAction *actionNew;
    QAction *actionOpen;
    QAction *actionQuit;
    QAction *actionAbout;
    QAction *actionMove_M;
    QAction *actionScale_M;
    QAction *actionRotate_M;
    QAction *actionSelect_M;
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *ogl_layout;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuTools;
    QMenu *menuHelp;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QToolBar *toolBar;
    QDockWidget *attr_editor;
    QWidget *dockWidgetContents;
    QVBoxLayout *verticalLayout;
    QSpacerItem *horizontalSpacer;

    void setupUi(QMainWindow *Barber)
    {
        if (Barber->objectName().isEmpty())
            Barber->setObjectName(QStringLiteral("Barber"));
        Barber->resize(691, 528);
        Barber->setStyleSheet(QStringLiteral(""));
        actionNew = new QAction(Barber);
        actionNew->setObjectName(QStringLiteral("actionNew"));
        actionOpen = new QAction(Barber);
        actionOpen->setObjectName(QStringLiteral("actionOpen"));
        actionQuit = new QAction(Barber);
        actionQuit->setObjectName(QStringLiteral("actionQuit"));
        actionAbout = new QAction(Barber);
        actionAbout->setObjectName(QStringLiteral("actionAbout"));
        actionMove_M = new QAction(Barber);
        actionMove_M->setObjectName(QStringLiteral("actionMove_M"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/move_M.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionMove_M->setIcon(icon);
        actionScale_M = new QAction(Barber);
        actionScale_M->setObjectName(QStringLiteral("actionScale_M"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/icons/scale_M.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionScale_M->setIcon(icon1);
        actionRotate_M = new QAction(Barber);
        actionRotate_M->setObjectName(QStringLiteral("actionRotate_M"));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/icons/rotate_M.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionRotate_M->setIcon(icon2);
        actionSelect_M = new QAction(Barber);
        actionSelect_M->setObjectName(QStringLiteral("actionSelect_M"));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/icons/aselect.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSelect_M->setIcon(icon3);
        centralWidget = new QWidget(Barber);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        ogl_layout = new QVBoxLayout();
        ogl_layout->setSpacing(6);
        ogl_layout->setObjectName(QStringLiteral("ogl_layout"));

        horizontalLayout->addLayout(ogl_layout);

        Barber->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(Barber);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 691, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName(QStringLiteral("menuEdit"));
        menuTools = new QMenu(menuBar);
        menuTools->setObjectName(QStringLiteral("menuTools"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QStringLiteral("menuHelp"));
        Barber->setMenuBar(menuBar);
        mainToolBar = new QToolBar(Barber);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        Barber->addToolBar(Qt::LeftToolBarArea, mainToolBar);
        statusBar = new QStatusBar(Barber);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        Barber->setStatusBar(statusBar);
        toolBar = new QToolBar(Barber);
        toolBar->setObjectName(QStringLiteral("toolBar"));
        Barber->addToolBar(Qt::TopToolBarArea, toolBar);
        attr_editor = new QDockWidget(Barber);
        attr_editor->setObjectName(QStringLiteral("attr_editor"));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QStringLiteral("dockWidgetContents"));
        verticalLayout = new QVBoxLayout(dockWidgetContents);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalSpacer = new QSpacerItem(106, 20, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

        verticalLayout->addItem(horizontalSpacer);

        attr_editor->setWidget(dockWidgetContents);
        Barber->addDockWidget(static_cast<Qt::DockWidgetArea>(2), attr_editor);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuTools->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionNew);
        menuFile->addAction(actionOpen);
        menuFile->addSeparator();
        menuFile->addAction(actionQuit);
        menuHelp->addAction(actionAbout);
        mainToolBar->addAction(actionSelect_M);
        mainToolBar->addAction(actionMove_M);
        mainToolBar->addAction(actionRotate_M);
        mainToolBar->addAction(actionScale_M);

        retranslateUi(Barber);

        QMetaObject::connectSlotsByName(Barber);
    } // setupUi

    void retranslateUi(QMainWindow *Barber)
    {
        Barber->setWindowTitle(QApplication::translate("MainWindowClass", "MainWindow", 0));
        actionNew->setText(QApplication::translate("MainWindowClass", "New", 0));
        actionNew->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+N", 0));
        actionOpen->setText(QApplication::translate("MainWindowClass", "Open", 0));
        actionOpen->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+O", 0));
        actionQuit->setText(QApplication::translate("MainWindowClass", "Quit", 0));
        actionQuit->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+Q", 0));
        actionAbout->setText(QApplication::translate("MainWindowClass", "About", 0));
        actionMove_M->setText(QApplication::translate("MainWindowClass", "move_M", 0));
#ifndef QT_NO_TOOLTIP
        actionMove_M->setToolTip(QApplication::translate("MainWindowClass", "Move Tool", 0));
#endif // QT_NO_TOOLTIP
        actionScale_M->setText(QApplication::translate("MainWindowClass", "scale_M", 0));
#ifndef QT_NO_TOOLTIP
        actionScale_M->setToolTip(QApplication::translate("MainWindowClass", "Scale Tool", 0));
#endif // QT_NO_TOOLTIP
        actionRotate_M->setText(QApplication::translate("MainWindowClass", "rotate_M", 0));
#ifndef QT_NO_TOOLTIP
        actionRotate_M->setToolTip(QApplication::translate("MainWindowClass", "Rotate Tool", 0));
#endif // QT_NO_TOOLTIP
        actionSelect_M->setText(QApplication::translate("MainWindowClass", "select_M", 0));
#ifndef QT_NO_TOOLTIP
        actionSelect_M->setToolTip(QApplication::translate("MainWindowClass", "Select Tool", 0));
#endif // QT_NO_TOOLTIP
        menuFile->setTitle(QApplication::translate("MainWindowClass", "File", 0));
        menuEdit->setTitle(QApplication::translate("MainWindowClass", "Edit", 0));
        menuTools->setTitle(QApplication::translate("MainWindowClass", "Tools", 0));
        menuHelp->setTitle(QApplication::translate("MainWindowClass", "Help", 0));
        toolBar->setWindowTitle(QApplication::translate("MainWindowClass", "toolBar", 0));
        attr_editor->setWindowTitle(QApplication::translate("MainWindowClass", "Attribute Editor", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindowClass: public Ui_MainWindowClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
