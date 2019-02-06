import React from "react";

class Arrow extends React.Component {
    constructor(props) {
        super(props);
    }
    render() {
        var path = "M10 15 L20 20 L30 15";
        const { dir } = this.props;
        switch (dir) {
            case "up":
                path = "M10 20 L20 15 L30 20";
                break;
            default:
                break;
        }
        return (
            <svg width="40" height="40" viewBox="0 0 40 40">
                <path d={path}
                    style={{ stroke: "#424242", fill: "transparent", strokeWidth: "3px" }} />
                />
                </svg>
            )
    }

}
export default Arrow;