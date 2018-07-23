import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import "../components"

Page {
    id: tournamentNavigationPage

    ColumnLayout {
        id: pageLayout
        spacing: 10
        anchors.fill: parent
        anchors.margins: 15

        Rectangle {
            id: tournamentHeader
            color: mainWindow.colorA
            radius: 5

            Layout.fillWidth: true
            Layout.preferredHeight: 50

            Text {
                id: tournamentName
                text: currentTournament.name
                color: mainWindow.fontColor
                width: parent.width * 0.5
                font.pointSize: 23
                font.bold: true
                elide: Text.ElideRight
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 5
            }

            Text {
                id: hostName
                text: qsTr("by ") + currentTournament.hostName
                color: mainWindow.fontColor
                opacity: 0.5
                width: parent.width * 0.5
                font.pointSize: 10
                elide: Text.ElideRight
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.leftMargin: 10
                anchors.bottomMargin: 2
            }

            IconButton {
                id: closeTournamentPageButton
                width: height
                iconSource: "qrc://assets/icons/icons/icons8_Close_Window.png"
                margins: 3
                marginsOnPressed: 6
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.margins: 5

                onClicked: navigationPage.popTournament()
            }

            ToolTipIcon {
                id: tournamentInfoToolTip
                width: height
                iconSource: "qrc://assets/icons/icons/icons8_Question_Mark.png"
                text: "Password Required: ...\n" +
                      "Entries End Time: ...\n" +
                      "Typers: ..."
                margins: 3
                marginsOnHovered: 6
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: closeTournamentPageButton.left
                anchors.topMargin: 5
                anchors.bottomMargin: 5
                anchors.rightMargin: 2
            }

            PopupList {
                id: roundsList
                width: 250
                color: mainWindow.colorB
                radius: 10
                textColor: mainWindow.fontColor
                fontSize: 18
                buttonColor: mainWindow.backgroundColor
                buttonIcon: "qrc://assets/icons/icons/icons8_Expand_Arrow.png"
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: tournamentInfoToolTip.left
                anchors.topMargin: 5
                anchors.bottomMargin: 5
                anchors.rightMargin: 2

                Component.onCompleted: roundsList.addItem(qsTr("Leaderboards"))
            }
        }

        StackView {
           id: tournamentView
           initialItem: TournamentLeaderbordsPage {}

           Layout.fillHeight: true
           Layout.fillWidth: true
        }
    }

    Component {
        id: addRoundButtonItem

        IconButton {
            width: height
            iconSource: "qrc://assets/icons/icons/icons8_Add_New.png"
            margins: 3
            marginsOnPressed: 6
        }
    }

    Component {
        id: tournamentClosedToolTipItem

        ToolTipIcon {
            width: height
            iconSource: "qrc://assets/icons/icons/icons8_Lock.png"
            text: "This tournament has come to an end."
            margins: 3
            marginsOnHovered: 6
        }
    }

    Component {
        id: finishTournamentButtonItem

        IconButton {
            width: height
            iconSource: "qrc://assets/icons/icons/icons8_Padlock.png"
            margins: 3
            marginsOnPressed: 6

            onClicked: finishTournamentPopup.open()
        }
    }

    Item {
        width: finishTournamentPopup.width
        height: finishTournamentPopup.height
        anchors.centerIn: parent

        PopupBox {
            id: finishTournamentPopup
            width: 500
            height: 300

            Text {
                text: qsTr("Finishing Tournament")
                color: mainWindow.fontColor
                font.bold: true
                font.pointSize: 25
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 15
            }

            TextEdit {
                text: qsTr("You will not be able to reopen this tournament! Do you really want to finish it?")
                font.pointSize: 12
                color: mainWindow.fontColor
                readOnly: true
                wrapMode: TextEdit.Wrap
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.leftMargin: 70
                anchors.rightMargin: 70
            }

            Row {
                spacing: 5
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter

                Button {
                    id: yesButton
                    text: qsTr("Yes")
                    width: 150
                    font.pointSize: 20
                    font.bold: true

                    onClicked: {
                        finishTournamentPopup.close()

                        if(currentUser.username === currentTournament.hostName)
                            backend.finishTournament(currentTournament.name, currentTournament.hostName)
                        else
                            navigationPage.showResponse(qsTr("You are not the host of this tournament!"))
                    }
                }

                Button {
                    id: noButton
                    text: qsTr("No")
                    width: 150
                    font.pointSize: 20
                    font.bold: true

                    onClicked: finishTournamentPopup.close()
                }
            }
        }
    }

    Component.onCompleted: backend.downloadTournamentInfo(currentTournament.name, currentTournament.hostName)

    Connections {
        target: packetProcessor

        onTournamentInfoDownloadReply: {
            tournamentInfoToolTip.text = "Password Required: " + tournamentInfo[0] + "\n" +
                                         "Entries End Time: " + tournamentInfo[1] + "\n" +
                                         "Typers: " + tournamentInfo[2] + "/" + tournamentInfo[3]

            currentTournament.passwordRequired = tournamentInfo[0] === "Yes" ? true : false
            currentTournament.entriesEndTime = Date.fromLocaleString(Qt.locale(), tournamentInfo[1], "dd.MM.yyyy hh:mm")
            currentTournament.typersNumber = parseInt(tournamentInfo[2])
            currentTournament.typersLimit = parseInt(tournamentInfo[3])

            if(currentTournament.hostName === currentUser.username)
                enableHostTools(opened)
            if(!opened)
                createTournamentClosedToolTip()
        }

        onTournamentRoundNameArrived: roundsList.addItem(name)
    }

    function enableHostTools(opened)
    {
        createAddRoundButton()

        if(opened)
            createTournamentCloseButton()
    }

    function createAddRoundButton()
    {
        var addRoundButton = addRoundButtonItem.createObject(tournamentHeader)
        addRoundButton.anchors.right = tournamentInfoToolTip.left
        addRoundButton.anchors.top = tournamentHeader.top
        addRoundButton.anchors.bottom = tournamentHeader.bottom
        addRoundButton.anchors.rightMargin = -6

        roundsList.anchors.right = addRoundButton.left
        roundsList.anchors.rightMargin = 0
    }

    function createTournamentClosedToolTip()
    {
        var tournamentClosedToolTip = tournamentClosedToolTipItem.createObject(tournamentHeader)
        tournamentClosedToolTip.anchors.right = roundsList.left
        tournamentClosedToolTip.anchors.top = tournamentHeader.top
        tournamentClosedToolTip.anchors.bottom = tournamentHeader.bottom
        tournamentClosedToolTip.anchors.rightMargin = 5
        tournamentClosedToolTip.anchors.topMargin = 5
        tournamentClosedToolTip.anchors.bottomMargin = 5
    }

    function createTournamentCloseButton()
    {
        var closeTournamentButton = finishTournamentButtonItem.createObject(tournamentHeader)
        closeTournamentButton.anchors.right = roundsList.left
        closeTournamentButton.anchors.top = tournamentHeader.top
        closeTournamentButton.anchors.bottom = tournamentHeader.bottom
        closeTournamentButton.anchors.rightMargin = 5
        closeTournamentButton.anchors.topMargin = 5
        closeTournamentButton.anchors.bottomMargin = 5
    }
}
