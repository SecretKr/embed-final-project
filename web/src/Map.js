import "./styles.css";
import "leaflet/dist/leaflet.css";
import { useEffect, useState } from "react";
import { MapContainer, TileLayer, Marker } from "react-leaflet";
import MarkerClusterGroup from "react-leaflet-cluster";
import { divIcon, point } from "leaflet";
import { initializeApp } from "firebase/app";
import { getDatabase, onValue, ref } from "firebase/database";
import firebaseConfig from './config';

const containerStyle = {
  position: 'flex',
  width: '80%',
  height: '100%'
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
const db = getDatabase(app);

// Function to create custom icon with PM value
const createCustomIcon = (pm) => {
  return new divIcon({
    html: `<span class="custom-icon">${pm}</span>`,
    className: "custom-icon",
    iconSize: point(38, 38, true),
  });
};

// Custom cluster icon function to show average PM value
const createClusterCustomIcon = (cluster) => {
  const markers = cluster.getAllChildMarkers();
  const totalPm = markers.reduce((acc, marker) => acc + parseFloat(marker.options.pm), 0);
  const avgPm = (totalPm / markers.length).toFixed(2);

  return new divIcon({
    html: `<span class="cluster-icon">${avgPm}</span>`,
    className: "custom-marker-cluster",
    iconSize: point(33, 33, true),
  });
};

const Map = ({ closeAndShowInformation }) => {
  const [markers, setMarkers] = useState([]);

  useEffect(() => {
    const fetchMarkers = () => {
      onValue(ref(db, 'logs'), (snapshot) => {
        const list = [];
        snapshot.forEach((doc) => {
          const lat = parseFloat(doc.val().lat);
          const lon = parseFloat(doc.val().lon);
          const pm = doc.val().pm;
          if (!isNaN(lat) && !isNaN(lon)) {
            list.push({
              idx: list.length + 1,
              geocode: [lat, lon],
              pm,
            });
          } else {
            console.error("Invalid latitude or longitude value:", doc.val());
          }
        });
        setMarkers(list);
      });
    };
    fetchMarkers();
  }, []);

  return (
    <MapContainer mapContainerStyle={containerStyle} center={[13.73826, 100.53241]} zoom={13}>
      <TileLayer
        attribution='&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
        url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
      />
      <MarkerClusterGroup
        chunkedLoading
        iconCreateFunction={createClusterCustomIcon}
      >
        {markers.map((marker) => (
          <Marker
            key={marker.geocode.toString()}
            position={marker.geocode}
            icon={createCustomIcon(marker.pm)}
            pm={marker.pm}
            eventHandlers={{
              click: () => closeAndShowInformation(marker.idx),
            }}
          />
        ))}
      </MarkerClusterGroup>
    </MapContainer>
  );
};

export default Map;
