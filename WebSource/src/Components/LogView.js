import React from "react";
import Button from "./Button"
import { getHomeUrl, getConfigData, getBaseuri } from "./utils"
class LogView extends React.Component {
    constructor(props) {
        super(props);
        this.API = getBaseuri() + "/get_log";
        this.state = { logtext: "" };
        this.linesToPar = this.linesToPar.bind(this);
        this.readData = this.readData.bind(this);
        this.timerId = 0;
    }
    componentDidMount() {
        this.readData();
        this.timerId= setInterval(() => {
            this.readData();
        }, 5000);
    }
    componentWillUnmount() {
        clearInterval(this.timerId);
    }
    readData() {
        const readAllChunks =(readableStream) =>{
            const reader = readableStream.getReader();
            const chunks = [];
            var stringval = "";
            function pump() {
                return reader.read().then(({ value, done }) => {
                    if (done) {
                        //return chunks;
                        return  stringval;
                    }
                   
                    stringval = value.reduce((acc, item) => { acc += String.fromCharCode(item); return acc; }, stringval);
                   
                    //chunks.push(value);
                    return pump();
                });
            }

            return pump();
        }

        fetch(this.API)
            .then(response => {
             
                
                if (response.status === 200) {
                    console.log("read chunk");
                    return readAllChunks(response.body);
                }
                else {

                    console.log("Error get log");
                }
            }
            )
            
            .then(chunks => this.setState({ logtext: this.linesToPar(chunks) }))
            .catch(function (error) {
                
                console.log(error);
              
            });
    }
    linesToPar(node) {
       return node.split('\n').map(text => <p>{text}</p>);
    }
    render() {
        const style = { display: 'block', overflow: "auto", height: "400px", lineHeight: "0.2", fontSize:"x-small" }
        return (
            <div className="left-align" style={style}>
                {this.state.logtext}
            </div>
        )
    }
}
export default LogView;