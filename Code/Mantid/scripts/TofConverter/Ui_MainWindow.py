# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'CONVERTER.ui'
#
# Created: Fri Jul 20 11:31:01 2012
#      by: PyQt4 UI code generator 4.9.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName(_fromUtf8("MainWindow"))
        MainWindow.resize(461, 305)
        self.centralwidget = QtGui.QWidget(MainWindow)
        self.centralwidget.setObjectName(_fromUtf8("centralwidget"))
        self.groupBox = QtGui.QGroupBox(self.centralwidget)
        self.groupBox.setGeometry(QtCore.QRect(10, 0, 441, 261))
        self.groupBox.setObjectName(_fromUtf8("groupBox"))
        self.inputUnits = QtGui.QComboBox(self.groupBox)
        self.inputUnits.setGeometry(QtCore.QRect(230, 20, 191, 31))
        self.inputUnits.setObjectName(_fromUtf8("inputUnits"))
        self.inputUnits.addItem(_fromUtf8(""))
        self.inputUnits.addItem(_fromUtf8(""))
        self.inputUnits.addItem(_fromUtf8(""))
        self.inputUnits.addItem(_fromUtf8(""))
        self.inputUnits.addItem(_fromUtf8(""))
        self.inputUnits.addItem(_fromUtf8(""))
        self.inputUnits.addItem(_fromUtf8(""))
        self.inputUnits.addItem(_fromUtf8(""))
        self.inputUnits.addItem(_fromUtf8(""))
        self.inputUnits.addItem(_fromUtf8(""))
        self.groupBox_5 = QtGui.QGroupBox(self.groupBox)
        self.groupBox_5.setGeometry(QtCore.QRect(10, 120, 211, 61))
        self.groupBox_5.setObjectName(_fromUtf8("groupBox_5"))
        self.lineEdit_3 = QtGui.QLineEdit(self.groupBox_5)
        self.lineEdit_3.setGeometry(QtCore.QRect(10, 20, 191, 31))
        self.lineEdit_3.setObjectName(_fromUtf8("lineEdit_3"))
        self.groupBox_4 = QtGui.QGroupBox(self.groupBox)
        self.groupBox_4.setGeometry(QtCore.QRect(10, 60, 211, 61))
        self.groupBox_4.setObjectName(_fromUtf8("groupBox_4"))
        self.lineEdit_4 = QtGui.QLineEdit(self.groupBox_4)
        self.lineEdit_4.setGeometry(QtCore.QRect(10, 20, 191, 31))
        self.lineEdit_4.setObjectName(_fromUtf8("lineEdit_4"))
        self.convert = QtGui.QPushButton(self.groupBox)
        self.convert.setGeometry(QtCore.QRect(230, 60, 201, 121))
        self.convert.setObjectName(_fromUtf8("convert"))
        self.groupBox_2 = QtGui.QGroupBox(self.groupBox)
        self.groupBox_2.setGeometry(QtCore.QRect(10, 190, 421, 61))
        self.groupBox_2.setObjectName(_fromUtf8("groupBox_2"))
        self.lineEdit_2 = QtGui.QLineEdit(self.groupBox_2)
        self.lineEdit_2.setGeometry(QtCore.QRect(10, 20, 201, 31))
        self.lineEdit_2.setReadOnly(True)
        self.lineEdit_2.setObjectName(_fromUtf8("lineEdit_2"))
        self.outputUnits = QtGui.QComboBox(self.groupBox_2)
        self.outputUnits.setGeometry(QtCore.QRect(220, 20, 191, 31))
        self.outputUnits.setObjectName(_fromUtf8("outputUnits"))
        self.outputUnits.addItem(_fromUtf8(""))
        self.outputUnits.addItem(_fromUtf8(""))
        self.outputUnits.addItem(_fromUtf8(""))
        self.outputUnits.addItem(_fromUtf8(""))
        self.outputUnits.addItem(_fromUtf8(""))
        self.outputUnits.addItem(_fromUtf8(""))
        self.outputUnits.addItem(_fromUtf8(""))
        self.outputUnits.addItem(_fromUtf8(""))
        self.outputUnits.addItem(_fromUtf8(""))
        self.outputUnits.addItem(_fromUtf8(""))
        self.InputVal = QtGui.QLineEdit(self.groupBox)
        self.InputVal.setGeometry(QtCore.QRect(10, 19, 211, 31))
        self.InputVal.setObjectName(_fromUtf8("InputVal"))
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtGui.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 461, 20))
        self.menubar.setObjectName(_fromUtf8("menubar"))
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtGui.QStatusBar(MainWindow)
        self.statusbar.setObjectName(_fromUtf8("statusbar"))
        MainWindow.setStatusBar(self.statusbar)
        self.action7 = QtGui.QAction(MainWindow)
        self.action7.setObjectName(_fromUtf8("action7"))

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(QtGui.QApplication.translate("MainWindow", "TofConverter", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox.setTitle(QtGui.QApplication.translate("MainWindow", "CONVERTER", None, QtGui.QApplication.UnicodeUTF8))
        self.inputUnits.setItemText(0, QtGui.QApplication.translate("MainWindow", "Energy  (meV)", None, QtGui.QApplication.UnicodeUTF8))
        self.inputUnits.setItemText(1, QtGui.QApplication.translate("MainWindow", "Wavelength (Angstroms)", None, QtGui.QApplication.UnicodeUTF8))
        self.inputUnits.setItemText(2, QtGui.QApplication.translate("MainWindow", "Nu (THz)", None, QtGui.QApplication.UnicodeUTF8))
        self.inputUnits.setItemText(3, QtGui.QApplication.translate("MainWindow", "Velocity (m/s)", None, QtGui.QApplication.UnicodeUTF8))
        self.inputUnits.setItemText(4, QtGui.QApplication.translate("MainWindow", "Momentum ( k Angstroms^-1)", None, QtGui.QApplication.UnicodeUTF8))
        self.inputUnits.setItemText(5, QtGui.QApplication.translate("MainWindow", "Temperature (K)", None, QtGui.QApplication.UnicodeUTF8))
        self.inputUnits.setItemText(6, QtGui.QApplication.translate("MainWindow", "Energy (cm^-1)", None, QtGui.QApplication.UnicodeUTF8))
        self.inputUnits.setItemText(7, QtGui.QApplication.translate("MainWindow", "Momentum transfer (Q Angstroms^-1)", None, QtGui.QApplication.UnicodeUTF8))
        self.inputUnits.setItemText(8, QtGui.QApplication.translate("MainWindow", "d-Spacing (Angstroms)", None, QtGui.QApplication.UnicodeUTF8))
        self.inputUnits.setItemText(9, QtGui.QApplication.translate("MainWindow", "Time of flight (microseconds)", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox_5.setTitle(QtGui.QApplication.translate("MainWindow", "Total flight path (meters)", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox_4.setTitle(QtGui.QApplication.translate("MainWindow", "Scattering Angle (degrees)", None, QtGui.QApplication.UnicodeUTF8))
        self.convert.setText(QtGui.QApplication.translate("MainWindow", "CONVERT", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox_2.setTitle(QtGui.QApplication.translate("MainWindow", "OUTPUT", None, QtGui.QApplication.UnicodeUTF8))
        self.outputUnits.setItemText(0, QtGui.QApplication.translate("MainWindow", "Energy  (meV)", None, QtGui.QApplication.UnicodeUTF8))
        self.outputUnits.setItemText(1, QtGui.QApplication.translate("MainWindow", "Wavelength (Angstroms)", None, QtGui.QApplication.UnicodeUTF8))
        self.outputUnits.setItemText(2, QtGui.QApplication.translate("MainWindow", "Nu (THz)", None, QtGui.QApplication.UnicodeUTF8))
        self.outputUnits.setItemText(3, QtGui.QApplication.translate("MainWindow", "Velocity (m/s)", None, QtGui.QApplication.UnicodeUTF8))
        self.outputUnits.setItemText(4, QtGui.QApplication.translate("MainWindow", "Momentum ( k Angstroms^-1)", None, QtGui.QApplication.UnicodeUTF8))
        self.outputUnits.setItemText(5, QtGui.QApplication.translate("MainWindow", "Temperature (K)", None, QtGui.QApplication.UnicodeUTF8))
        self.outputUnits.setItemText(6, QtGui.QApplication.translate("MainWindow", "Energy (cm^-1)", None, QtGui.QApplication.UnicodeUTF8))
        self.outputUnits.setItemText(7, QtGui.QApplication.translate("MainWindow", "Momentum transfer (Q Angstroms^-1)", None, QtGui.QApplication.UnicodeUTF8))
        self.outputUnits.setItemText(8, QtGui.QApplication.translate("MainWindow", "d-Spacing (Angstroms)", None, QtGui.QApplication.UnicodeUTF8))
        self.outputUnits.setItemText(9, QtGui.QApplication.translate("MainWindow", "Time of flight (microseconds)", None, QtGui.QApplication.UnicodeUTF8))
        self.action7.setText(QtGui.QApplication.translate("MainWindow", "7", None, QtGui.QApplication.UnicodeUTF8))
