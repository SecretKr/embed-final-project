import React, { useCallback, useState, useEffect } from 'react';
import { GoogleMap, useJsApiLoader } from '@react-google-maps/api';
import { initializeApp } from "firebase/app"
import { getDatabase, onValue, ref } from "firebase/database";
import firebaseConfig from './config';
import DustScoreSuperMarker from './DustScoreSuperMarker'; // Import the new supercluster-based marker component

const containerStyle = {
  position: 'flex',
  width: '80%',
  height: '100%'
};

const center = {
  lat: 13.73826,
  lng: 100.53241
};

function Map() {
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
      setData(list);
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

  return isLoaded ? (
    <GoogleMap
      mapContainerStyle={containerStyle}
      center={center}
      zoom={10}
      onLoad={onLoad}
      onUnmount={onUnmount}
    >
      {/* Replace direct rendering of markers with DustScoreSuperMarker */}
      <DustScoreSuperMarker markers={markers} zoom={10} />
    </GoogleMap>
  ) : <></>;
}

export default React.memo(Map);
