import React from 'react';
import DustScoreMarker from './DustScoreMarker';

const DustScoreSuperMarker = ({ markers , closeAndShowInformation}) => {
  // Render DustScoreMarker components
  const renderMarkers = () => {
    return markers.map((marker, index) => (
      <DustScoreMarker
        key={index}
        position={{ lat: marker.lat, lng: marker.lng }}
        index={index}
        pm={marker.pm}
        closeAndShowInformation={closeAndShowInformation}
      />
    ));
  };

  return renderMarkers();
};

export default DustScoreSuperMarker;
