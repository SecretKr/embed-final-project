import React, { useState, useEffect } from 'react';
import { initializeApp } from "firebase/app";
import { getDatabase, onValue, ref } from "firebase/database";
import firebaseConfig from './config';
import './App.css';
import Map from './Map';

const App = () => {
  const app = initializeApp(firebaseConfig);
  const db = getDatabase(app);
  const [data, setData] = useState([]);
  const [currentOpenBox, setCurrentOpenBox] = useState();

  const createInformationBox = (doc, index) => {
    const informationBox = document.createElement("div");
    informationBox.className = "information_box";
    informationBox.id = `information_box_${index}`;
    informationBox.style.display = "none";

    const createParagraph = (textContent) => {
      const p = document.createElement("p");
      p.textContent = textContent;
      return p;
    };

    informationBox.appendChild(createParagraph(`Location: ${index}`));
    informationBox.appendChild(createParagraph(`Timestamp: ${new Date(Number(doc.key * 1000)).toLocaleString('en-us')}`));
    informationBox.appendChild(createParagraph(`Latitude: ${doc.val().lat}`));
    informationBox.appendChild(createParagraph(`Longitude: ${doc.val().lon}`));
    informationBox.appendChild(createParagraph(`PM Value: ${doc.val().pm}`));

    return informationBox;
  };

  useEffect(() => {
    const informationBoxContainer = document.querySelector(".information_box_container");

    onValue(ref(db, 'logs'), (snapshot) => {
      const list = [];
      snapshot.forEach((doc) => {
        const index = list.length + 1;
        list.push({
          index,
          timestamp: doc.key,
          lat: doc.val().lat,
          lon: doc.val().lon,
          pm: doc.val().pm
        });

        const informationBox = createInformationBox(doc, index);
        informationBoxContainer.appendChild(informationBox);
      });
      setData(list);
    });
  }, [db]);

  const closeAndShowInformation = (index) => {
    if (currentOpenBox && currentOpenBox !== index) {
      const currentBox = document.getElementById(`information_box_${currentOpenBox}`);
      if (currentBox) {
        currentBox.style.display = "none";
      }
    }

    const informationBox = document.getElementById(`information_box_${index}`);
    if (informationBox) {
      const currentDisplay = window.getComputedStyle(informationBox).getPropertyValue('display');
      informationBox.style.display = currentDisplay === 'none' ? 'flex' : 'none';
      setCurrentOpenBox(currentDisplay === 'none' ? index : null);
    } else {
      console.error(`Information box with ID information_box_${index} not found.`);
    }
  };

  return (
    <div className="background">
      <div className="information">
        <div className="title_information">
          <p>Information</p>
        </div>
        <div className="information_box_container"></div>
        <div className="name">
          <p>กรณ์ สุรพัฒน์ 6531301721</p>
          <p>กิตติพัฒน์ จูสิงห์ 6531304621</p>
          <p>ไกรวิชญ์ จรสโรจน์กุล 6532026321</p>
          <p>ศุภวิชญ์ เจนสวัสดิ์พงศ์ 6532175121</p>
        </div>
      </div>

      <div className="page">
        <div className="title_page">
          <p>Embedded System Laboratory Final Project</p>
        </div>
        <div className="map_container">
          <Map closeAndShowInformation={closeAndShowInformation}/>
          <div className="slider">
            <div className="slide_container">
              {data.map((item) => (
                <button key={item.index} className="button" onClick={() => closeAndShowInformation(item.index)}>
                  <p className="ll">Location </p>{item.index}
                </button>
              ))}
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};

export default App;
