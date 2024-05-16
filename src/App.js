import React, { useState, useEffect } from 'react';
//Wit's--------------------------------------------------
import { initializeApp } from "firebase/app"
import { getDatabase, onValue, ref } from "firebase/database";
import firebaseConfig from './config';
//--------------------------------------------------
import './App.css';
import Map from './Map';

const App = () => {
  
  //Wit's--------------------------------------------------
  const app = initializeApp(firebaseConfig);
  const db = getDatabase(app);
  const [data, setData] = useState([]);

  useEffect(() => {
    onValue(ref(db, 'logs'), (snapshot) => {
      const list = [];
      snapshot.forEach((doc) => {
        list.push({index: list.length + 1, timestamp: doc.key, lat: doc.val().lat, lon: doc.val().lon, pm: doc.val().pm})

        const informationBoxContainer = document.querySelector(".information_box_container");
        const informationBox = document.createElement("div");
        informationBox.className = "information_box";
        informationBox.id = `information_box_${list.length}`;
        informationBox.style.display = "none";
        const location = document.createElement("p");
        const timestamp = document.createElement("p");
        const latitude = document.createElement("p");
        const longitude = document.createElement("p");
        const pmValue = document.createElement("p");
        location.textContent = `Location: ${list.length}`;
        timestamp.textContent = `Timestamp: ${doc.key}`;
        latitude.textContent = `Latitude: ${doc.val().lat}`;
        longitude.textContent = `Longitude: ${doc.val().lon}`;
        pmValue.textContent = `PM Value: ${doc.val().pm}`;
        informationBox.appendChild(location);
        informationBox.appendChild(timestamp);
        informationBox.appendChild(latitude);
        informationBox.appendChild(longitude);
        informationBox.appendChild(pmValue);
        informationBoxContainer.appendChild(informationBox);

      })
      setData(list);
    })
  }, [db]);
  //--------------------------------------------------
  
  const closeAndShowInformation = (index, length) => {
    /*alert(`Button ${index} and length = ${length}`);*/
    for(var j = 1; j <= length; j++) {
      if (j !== index) {
        let tmp = document.getElementById(`information_box_${j}`);
        if (tmp.style.display === "flex") {
          tmp.style.display = "none";
          break;
        }
      }
    }
    let informationBox = document.getElementById(`information_box_${index}`);
    if (informationBox.style.display === "none") {
        informationBox.style.display = "flex";
    } else {
        informationBox.style.display = "none";
    }
  };

  return (
    <div className="background">
      
      <div className="information">
        <div className="title_information">
          <p>Information</p>
        </div>
        <div className="information_box_container">
        </div>
        <div className="name">
          <p>กรณ์ สุรพัฒน์ 6531301721</p>
          <p>กิตติพัฒน์ จูสิงห์ 6531304621</p>
          <p>ไกรวิชญ์ จรสโรจน์กุล 6531301721</p>
          <p>ศุภวิชญ์ เจนสวัสดิ์พงศ์ 6532175121</p>
        </div>
      </div>
      
      <div className="page">
        <div className="title_page">
            <p>Embedded System Laboratory Final Project</p>
        </div>
        <div className="map_container">
          <Map/>
          <div className="slider">
            <div className="slide_container">
              {data.map((item) => (
                <button key={item.index} className="button" onClick={() => closeAndShowInformation(item.index, data.length)}>Location {item.index}</button>
              ))}
            </div>
          </div>
        </div>
      </div>

    </div>

  );
};

export default App;