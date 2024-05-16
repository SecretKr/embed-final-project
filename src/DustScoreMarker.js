// src/DustScoreMarker.js
import React from 'react';

const DustScoreMarker = ({ lat, lng, text }) => {
  console.log("Rendering marker at: ", lat, lng, text); // Debug log
  return (
    <div style={{
      position: 'absolute',
      transform: 'translate(-50%, -50%)',
      color: 'white', 
      background: 'red',
      padding: '10px 15px',
      display: 'inline-flex',
      textAlign: 'center',
      alignItems: 'center',
      justifyContent: 'center',
      borderRadius: '50%'
    }}>
      {text}
    </div>
  );
};

export default DustScoreMarker;
