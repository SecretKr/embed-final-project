import { Marker } from '@react-google-maps/api';

const DustScoreMarker = ({ position, index, pm, closeAndShowInformation }) => {
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
            <circle cx="20" cy="20" r="18" fill="#d67018" />
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


export default DustScoreMarker;
