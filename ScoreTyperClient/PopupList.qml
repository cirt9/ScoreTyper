import QtQuick 2.9
import QtQuick.Controls 2.2

Rectangle {
    id: root

    readonly property alias currentItemText: listItemText.text
    property alias buttonColor: expandListButton.color
    property alias buttonIcon: expandListButton.iconSource
    property alias textColor: listItemText.color
    property alias fontSize: listItemText.font.pointSize
    readonly property alias currentIndex: list.currentIndex
    signal itemChanged()

    Text {
        id: listItemText
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: expandListButton.left
        anchors.leftMargin: parent.radius
        anchors.rightMargin: 3

        onTextChanged: root.itemChanged()
    }

    CheckableIconButton {
        id: expandListButton
        height: parent.height
        width: height
        radius: parent.radius
        radiusOnChecked: radius
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        onCheckedChanged: {
            if(checked)
                list.open()
            else
                list.close()
        }
    }

    Menu {
        id: list
        width: parent.width
        y: parent.height

        Component {
            id: listItem
            MenuItem {onClicked: listItemText.text = text}
        }

        onOpenedChanged: {
            if(!opened)
                expandListButton.checked = false
        }
    }

    function addItem(text)
    {
        list.addItem(listItem.createObject(root, {text: text }))

        if(list.count === 1)
            listItemText.text = text
    }
}
