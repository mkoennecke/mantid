# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui/reduction_main.ui'
#
# Created: Tue Sep 25 11:54:26 2012
#      by: PyQt4 UI code generator 4.7.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_SANSReduction(object):
    def setupUi(self, SANSReduction):
        SANSReduction.setObjectName("SANSReduction")
        SANSReduction.resize(1062, 989)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(SANSReduction.sizePolicy().hasHeightForWidth())
        SANSReduction.setSizePolicy(sizePolicy)
        self.centralwidget = QtGui.QWidget(SANSReduction)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.centralwidget.sizePolicy().hasHeightForWidth())
        self.centralwidget.setSizePolicy(sizePolicy)
        self.centralwidget.setAutoFillBackground(True)
        self.centralwidget.setObjectName("centralwidget")
        self.verticalLayout = QtGui.QVBoxLayout(self.centralwidget)
        self.verticalLayout.setObjectName("verticalLayout")
        self.tabWidget = QtGui.QTabWidget(self.centralwidget)
        self.tabWidget.setObjectName("tabWidget")
        self.tab = QtGui.QWidget()
        self.tab.setObjectName("tab")
        self.verticalLayout_2 = QtGui.QVBoxLayout(self.tab)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.label = QtGui.QLabel(self.tab)
        self.label.setObjectName("label")
        self.verticalLayout_2.addWidget(self.label)
        self.tabWidget.addTab(self.tab, "")
        self.verticalLayout.addWidget(self.tabWidget)
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.interface_chk = QtGui.QCheckBox(self.centralwidget)
        self.interface_chk.setEnabled(True)
        self.interface_chk.setObjectName("interface_chk")
        self.horizontalLayout.addWidget(self.interface_chk)
        spacerItem = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem)
        self.progress_bar = QtGui.QProgressBar(self.centralwidget)
        self.progress_bar.setProperty("value", 0)
        self.progress_bar.setObjectName("progress_bar")
        self.horizontalLayout.addWidget(self.progress_bar)
        spacerItem1 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem1)
        self.live_button = QtGui.QPushButton(self.centralwidget)
        self.live_button.setObjectName("live_button")
        self.horizontalLayout.addWidget(self.live_button)
        self.reduce_button = QtGui.QPushButton(self.centralwidget)
        self.reduce_button.setEnabled(True)
        self.reduce_button.setObjectName("reduce_button")
        self.horizontalLayout.addWidget(self.reduce_button)
        self.save_button = QtGui.QPushButton(self.centralwidget)
        self.save_button.setEnabled(True)
        self.save_button.setObjectName("save_button")
        self.horizontalLayout.addWidget(self.save_button)
        self.export_button = QtGui.QPushButton(self.centralwidget)
        self.export_button.setEnabled(True)
        self.export_button.setObjectName("export_button")
        self.horizontalLayout.addWidget(self.export_button)
        self.verticalLayout.addLayout(self.horizontalLayout)
        SANSReduction.setCentralWidget(self.centralwidget)
        self.menubar = QtGui.QMenuBar(SANSReduction)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 1062, 26))
        self.menubar.setObjectName("menubar")
        self.file_menu = QtGui.QMenu(self.menubar)
        self.file_menu.setObjectName("file_menu")
        self.tools_menu = QtGui.QMenu(self.menubar)
        self.tools_menu.setObjectName("tools_menu")
        SANSReduction.setMenuBar(self.menubar)
        self.statusbar = QtGui.QStatusBar(SANSReduction)
        self.statusbar.setObjectName("statusbar")
        SANSReduction.setStatusBar(self.statusbar)
        self.actionOpen = QtGui.QAction(SANSReduction)
        self.actionOpen.setObjectName("actionOpen")
        self.actionQuit = QtGui.QAction(SANSReduction)
        self.actionQuit.setObjectName("actionQuit")
        self.actionChange_Instrument = QtGui.QAction(SANSReduction)
        self.actionChange_Instrument.setObjectName("actionChange_Instrument")
        self.menubar.addAction(self.file_menu.menuAction())
        self.menubar.addAction(self.tools_menu.menuAction())

        self.retranslateUi(SANSReduction)
        self.tabWidget.setCurrentIndex(0)
        QtCore.QMetaObject.connectSlotsByName(SANSReduction)

    def retranslateUi(self, SANSReduction):
        SANSReduction.setWindowTitle(QtGui.QApplication.translate("SANSReduction", "SANS Reduction", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("SANSReduction", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:\'Ubuntu\'; font-size:11pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:14pt; font-weight:600;\">No instrument was selected!</span></p>\n"
"<p align=\"center\" style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"></p>\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt;\">You need to select an instrument from the Tools menu</span></p>\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt;\"> or the Instrument Dialog to continue.</span></p></body></html>", None, QtGui.QApplication.UnicodeUTF8))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab), QtGui.QApplication.translate("SANSReduction", "No Instrument Selected", None, QtGui.QApplication.UnicodeUTF8))
        self.interface_chk.setText(QtGui.QApplication.translate("SANSReduction", "Advanced interface", None, QtGui.QApplication.UnicodeUTF8))
        self.live_button.setToolTip(QtGui.QApplication.translate("SANSReduction", "Click to start reduction from the live stream", None, QtGui.QApplication.UnicodeUTF8))
        self.live_button.setText(QtGui.QApplication.translate("SANSReduction", "Live!", None, QtGui.QApplication.UnicodeUTF8))
        self.reduce_button.setToolTip(QtGui.QApplication.translate("SANSReduction", "Click to execute reduction.", None, QtGui.QApplication.UnicodeUTF8))
        self.reduce_button.setText(QtGui.QApplication.translate("SANSReduction", "Reduce", None, QtGui.QApplication.UnicodeUTF8))
        self.save_button.setToolTip(QtGui.QApplication.translate("SANSReduction", "Click to save your reduction parameters.", None, QtGui.QApplication.UnicodeUTF8))
        self.save_button.setText(QtGui.QApplication.translate("SANSReduction", "Save", None, QtGui.QApplication.UnicodeUTF8))
        self.export_button.setToolTip(QtGui.QApplication.translate("SANSReduction", "Click to export the reduction parameters to a python script that can be run in MantidPlot.", None, QtGui.QApplication.UnicodeUTF8))
        self.export_button.setText(QtGui.QApplication.translate("SANSReduction", "Export", None, QtGui.QApplication.UnicodeUTF8))
        self.file_menu.setTitle(QtGui.QApplication.translate("SANSReduction", "File", None, QtGui.QApplication.UnicodeUTF8))
        self.tools_menu.setTitle(QtGui.QApplication.translate("SANSReduction", "Tools", None, QtGui.QApplication.UnicodeUTF8))
        self.actionOpen.setText(QtGui.QApplication.translate("SANSReduction", "Open...", None, QtGui.QApplication.UnicodeUTF8))
        self.actionOpen.setToolTip(QtGui.QApplication.translate("SANSReduction", "Open a reduction settings file", None, QtGui.QApplication.UnicodeUTF8))
        self.actionOpen.setShortcut(QtGui.QApplication.translate("SANSReduction", "Ctrl+O", None, QtGui.QApplication.UnicodeUTF8))
        self.actionQuit.setText(QtGui.QApplication.translate("SANSReduction", "Quit", None, QtGui.QApplication.UnicodeUTF8))
        self.actionQuit.setShortcut(QtGui.QApplication.translate("SANSReduction", "Ctrl+Q, Ctrl+S", None, QtGui.QApplication.UnicodeUTF8))
        self.actionChange_Instrument.setText(QtGui.QApplication.translate("SANSReduction", "Change Instrument", None, QtGui.QApplication.UnicodeUTF8))
        self.actionChange_Instrument.setShortcut(QtGui.QApplication.translate("SANSReduction", "Ctrl+I", None, QtGui.QApplication.UnicodeUTF8))

