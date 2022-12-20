const urlApiStart = "./api/washstart"

async function getJSON() {
    const response = await fetch("./config/cycles.json");
    return await response.json();
};

async function processJson() {
    const sel = document.getElementById("washcycles");

    const json = await getJSON();
    json.washes.forEach(element => {
        var opt = document.createElement('option');
        opt.value = element.cycle;
        opt.innerHTML = element.cycle;
        sel.appendChild(opt);
    });

    changeWashSelection();

};

function toHoursAndMinutes(totalSeconds) {
    const totalMinutes = Math.floor(totalSeconds / 60);

    const seconds = totalSeconds % 60;
    const hours = Math.floor(totalMinutes / 60);
    const minutes = totalMinutes % 60;
    const rtn = (hours < 10 ? "0" + hours : hours) + ":" + (minutes < 10 ? "0" + minutes : minutes) + "." + (seconds < 10 ? "0" + seconds : seconds)
    return rtn;
}

async function changeWashSelection() {
    const sel = document.getElementById("washcycles");
    const tbl = document.getElementById("washtablebody");

    //Delete current rows.
    let tblRows = tbl.getElementsByTagName("tr");
    while (tblRows.length > 0) {
        tblRows[0].remove();
    }
    const json = await getJSON();

    json.washes.forEach(element => {
        //Itterate through selected config and add to table
        if (element.cycle == sel.value) {
            element.steps.forEach(elStep => {
                let newRow = tbl.insertRow();
                newRow.insertCell().append(elStep.description ?? elStep.step);
                newRow.insertCell().append(toHoursAndMinutes(elStep.duration / 1000));
                newRow.insertCell().append("");

            });
            console.log(element);
        }
    });
}
async function startWashSelection() {
    const json = await getJSON();
    const sel = document.getElementById("washcycles");
    json.washes.forEach(element => {
        //Itterate through selected config and add to table
        if (element.cycle == sel.value) {
            console.log(element);
            let selectedwash = element;
            //Post json to the API startwash
        };

    });
    const response = await fetch(urlApiStart, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        }
    });

};



window.onload = function () {
    processJson();
}


