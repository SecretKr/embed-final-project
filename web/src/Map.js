import React, { useCallback, useState, useEffect } from 'react';
import { GoogleMap, useJsApiLoader, Marker } from '@react-google-maps/api';
import { initializeApp } from "firebase/app"
import { getDatabase, onValue, ref } from "firebase/database";
import firebaseConfig from './config';

const containerStyle = {
  width: '100%',
  height: '400px'
};

const center = {
  lat: 13.73826,
  lng: 100.53241
};

function Map() {
  const app = initializeApp(firebaseConfig);
  const db = getDatabase(app);
  const [data, setData] = useState([]);

  useEffect(() => {
    onValue(ref(db, 'logs'), (snapshot) => {
      const list = [];
      snapshot.forEach((doc) => {
        list.push({timestamp: doc.key, lat: doc.val().lat, lon: doc.val().lon, pm: doc.val().pm})
      })
      setData(list);
    })
  }, [db]);

  const { isLoaded } = useJsApiLoader({
    id: 'google-map-script',
    googleMapsApiKey: process.env.REACT_APP_GOOGLE_MAPS_API_KEY
  });

  const [map, setMap] = useState(null);
  const [markers, setMarkers] = useState([]);

  const onLoad = useCallback(function callback(map) {
    setMap(map);
  }, []);

  const onUnmount = useCallback(function callback(map) {
    setMap(null);
  }, []);

  const handleMapClick = (event) => {
    setMarkers(current => [
      ...current,
      {
        lat: event.latLng.lat(),
        lng: event.latLng.lng(),
        time: new Date(),
      }
    ]);
  };

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
      onClick={handleMapClick}
    >
      {markers.map((marker, index) => (
        <CustomMarker
          key={index}
          position={{ lat: marker.lat, lng: marker.lng }}
          index={index + 1}
        />
      ))}
    </GoogleMap></>
  ) : <></>;
}

const CustomMarker = ({ position, index }) => {
  return (
    <Marker
      position={position}
      icon={{
        url: 'data:image/svg+xml;charset=UTF-8,' + encodeURIComponent(`
          <svg xmlns="http://www.w3.org/2000/svg" width="40" height="40" viewBox="0 0 40 40">
            <circle cx="20" cy="20" r="18" fill="#FF0000" />
            <text x="50%" y="50%" dominant-baseline="middle" text-anchor="middle" fill="#FFFFFF" font-size="16">
              ${index}
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
