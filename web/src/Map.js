import React, { useCallback, useState, useEffect } from 'react';
import { GoogleMap, useJsApiLoader, Marker } from '@react-google-maps/api';
import { initializeApp } from "firebase/app"
import { getDatabase, onValue, ref } from "firebase/database";
import firebaseConfig from './config';

const containerStyle = {
  position: 'flex',
  width: '80%',
  height: '100%'
};

const center = {
  lat: 13.73826,
  lng: 100.53241
};

function Map({ closeAndShowInformation }) {
  const app = initializeApp(firebaseConfig);
  const db = getDatabase(app);
  const [data, setData] = useState([]);
  const [map, setMap] = useState(null);
  const [markers, setMarkers] = useState([]);

  useEffect(() => {
    onValue(ref(db, 'logs'), (snapshot) => {
      const list = [];
      snapshot.forEach((doc) => {
        const lat = parseFloat(doc.val().lat); // Ensure lat is parsed as a float
        const lon = parseFloat(doc.val().lon); // Ensure lon is parsed as a float
        const pm = doc.val().pm;
        if (!isNaN(lat) && !isNaN(lon)) { // Check if lat and lon are valid numbers
          list.push({ lat, lon, pm }); // Push valid data to the list
        } else {
          console.error("Invalid latitude or longitude value:", doc.val());
        }
      });
      //setData(list);
      // Update markers state with the new data
      setMarkers(list.map(item => ({ lat: item.lat, lng: item.lon , pm: item.pm})));
    });
  }, [db]);

  const { isLoaded } = useJsApiLoader({
    id: 'google-map-script',
    googleMapsApiKey: process.env.REACT_APP_GOOGLE_MAPS_API_KEY
  });

  const onLoad = useCallback(function callback(map) {
    setMap(map);
  }, []);

  const onUnmount = useCallback(function callback(map) {
    setMap(null);
  }, []);

  return isLoaded ? (<>
    <div>
      {data.map((item) => (
        <p key={item.timestamp}>lat: {item.lat}, lon: {item.lon}, pm: {item.pm}</p>
      ))}
    </div>
    <GoogleMap
      mapContainerStyle={containerStyle}
      center={center}
      zoom={10}
      onLoad={onLoad}
      onUnmount={onUnmount}
    >
      {markers.map((marker, index) => (
        <CustomMarker
        key={index}
        position={{ lat: marker.lat, lng: marker.lng }}
        index={index + 1}
        pm={marker.pm}
        closeAndShowInformation={closeAndShowInformation}
      />
      ))}
    </GoogleMap></>
  ) : <></>;
}

const CustomMarker = ({ position, index, pm, closeAndShowInformation }) => {
  const handleMarkerClick = () => {
    closeAndShowInformation(index);
  };

  return (
    <Marker
      position={position}
      onClick={handleMarkerClick}
      icon={{
        url: 'data:image/svg+xml;charset=UTF-8,' + encodeURIComponent(`
          <svg xmlns="http://www.w3.org/2000/svg" width="40" height="40" viewBox="0 0 40 40">
            <circle cx="20" cy="20" r="18" fill="#FF0000" />
            <text x="50%" y="50%" dominant-baseline="middle" text-anchor="middle" fill="#FFFFFF" font-size="16">
              ${pm}
            </text>
          </svg>
        `),
        scaledSize: new window.google.maps.Size(40, 40),
        anchor: new window.google.maps.Point(20, 20),
      }}
    />
  );
};


export default React.memo(Map);
