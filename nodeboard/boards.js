// boards.js - status board information
const fs = require('fs');

var boardsdata = [
    {
        numval: 0x00,
        source: "whatever",
        labels: [
            "First Aid Kits",
            "Laser Cutter",
            "Table Saw",
            "Bandsaw 1",
            "Fridge",
            "Knife Switch",
            "Server 1",
            "Belt Sander",
        ],
    },
    {
        numval: 0x00,
        source: "whatever",
        labels: [
            "Not Set",
            "Not Set",
            "Not Set",
            "Not Set",
            "Not Set",
            "Not Set",
            "Not Set",
            "Not Set",
        ],
    },
];

function checkBoardIndex(idx) {
    if(!Array.isArray(boardsdata)) 
        return false;
    if (Number.isNaN(idx) || idx < 0 || idx >= boardsdata.length)
        return false;
    return true;
}

function getBoardVal(idx) {
    if(!checkBoardIndex(idx)) return undefined;
    return boardsdata[idx].numval;
}

function setBoardVal(idx, newval) {
    if(!checkBoardIndex(idx)) return undefined;
    boardsdata[idx].numval = newval;
}

function getBoardLabels(idx) {
    if(!checkBoardIndex(idx)) return undefined;
    return boardsdata[idx].labels;
}

function load() {
    try {
        var s = fs.readFileSync("boards.json");
        var d = JSON.parse(s);
        console.log("data is: - ");
        console.log(d);
        if(Array.isArray(d)){
            boardsdata = d; // TODO proper checking!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        } else throw "whut?";
    } catch (e) {
        console.log(e.message);
        console.log("Resort to using default boards!");
        save();
    }
}

function save() {
    try {
        console.log("saving boards to disk");
        var d = JSON.stringify(boardsdata);
        fs.writeFileSync("boards.json", d);
    } catch (e) {
        console.log(e.message);
        console.log("failed to save boards!");
    }
}

exports.checkBoardIndex = checkBoardIndex;
exports.getBoardVal = getBoardVal;
exports.setBoardVal = setBoardVal;
exports.getBoardLabels = getBoardLabels;
exports.load = load;
exports.save = save;

