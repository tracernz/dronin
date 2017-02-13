var headless = installer.environmentVariable("CI").toLowerCase() == "true" ? true : false

function Controller() {
    if (headless) {
        installer.autoRejectMessageBoxes();
        installer.installationFinished.connect(function() {
            gui.clickButton(buttons.FinishButton);
        })
    }
}

Controller.prototype.WelcomePageCallback = function() {
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.CredentialsPageCallback = function() {
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.IntroductionPageCallback = function() {
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.TargetDirectoryPageCallback = function()
{
    if (!installer.containsValue('dronin_qt_path')) {
        console.log("ERROR: dronin_qt_path value missing!")
        var warningLabel = gui.currentPageWidget().WarningLabel;
        warningLabel.setText("ERROR: dronin_qt_path value missing!");
        if (headless) {
            installer.autoAcceptMessageBoxes();
            gui.clickButton(buttons.CancelButton);
        }
    } else {
        gui.currentPageWidget().TargetDirectoryLineEdit.setText(installer.value('dronin_qt_path'));
        gui.currentPageWidget().TargetDirectoryLineEdit.setEnabled(false);
        gui.currentPageWidget().BrowseDirectoryButton.setEnabled(false);
        if (headless)
            gui.clickButton(buttons.NextButton);
    }
}

Controller.prototype.ComponentSelectionPageCallback = function() {
    if (installer.containsValue('dronin_qt_components')) {
        if (installer.isInstaller())
            gui.currentPageWidget().deselectAll();

        installer.value('dronin_qt_components').split(" ").forEach(function(component) {
            gui.currentPageWidget().selectComponent(component);
        });
    }
    if (headless)
        gui.clickButton(buttons.NextButton);
}

Controller.prototype.LicenseAgreementPageCallback = function() {
    if (headless) {
        gui.currentPageWidget().AcceptLicenseRadioButton.setChecked(true);
        gui.clickButton(buttons.NextButton);
    }
}

Controller.prototype.StartMenuDirectoryPageCallback = function() {
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.ReadyForInstallationPageCallback = function()
{
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.FinishedPageCallback = function() {
    var checkBoxForm = gui.currentPageWidget().LaunchQtCreatorCheckBoxForm
    if (checkBoxForm && checkBoxForm.launchQtCreatorCheckBox) {
        checkBoxForm.launchQtCreatorCheckBox.checked = false;
    }
    gui.clickButton(buttons.FinishButton);
}
